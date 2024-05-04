#include <mqueue.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "comm.h"
#include "servidor.h"
#include "libserver.h"

#define MAX_LINE_LEN 255
#define MAX_THREADS    10
#define MAX_PETICIONES    256
#define ERROR 0

mqd_t q_servidor;
pthread_mutex_t mutex_mensaje;
pthread_mutex_t mutex_file;
pthread_cond_t cond_mensaje;
pthread_cond_t cond_file;

void *funcion_hilo(void *arg) {
    int sc = *(int *) arg;
    free(arg);

    int ret;
    int resp;
    char usuario[256];
    char filename[256];
    char descripcion[256];
    char command[256];

    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    getpeername(sc, (struct sockaddr *)&addr, &addr_size);
    char *ip = inet_ntoa(addr.sin_addr);
    int port = ntohs(addr.sin_port);

    //Leer comando
    while(1){
        ret = readLine(sc, command, sizeof(command));
        if (ret <= 0) {
            printf("s> Error en recepción de comando o comando no soportado\n");
            break;
            //pthread_exit(NULL);
        }

        if (strcmp(command, "QUIT") == 0) {
            break;
        }
        // Leer nombre usuario
        if (readLine(sc, usuario, sizeof(usuario)) < 0) {
            printf("s> Error en recepción de username\n");
            close(sc);
            pthread_exit(NULL);
        }
        printf("s> OPERACION FROM: %s\n", usuario);

        ret = readLine(sc, filename, 256);
        if (ret < 0) {
            printf("Error en recepción op\n");
            pthread_exit(NULL);
        }

        ret = readLine(sc, descripcion, 256);
        if (ret < 0) {
            printf("Error en recepción op\n");
            pthread_exit(NULL);
        }

        if (strcmp(command, "REGISTER") == 0) {resp = register_user(usuario); }
        else if (strcmp(command, "UNREGISTER") == 0) { resp = unregister_user(usuario); }
        else if (strcmp(command, "CONNECT") == 0) { resp = connect_user(usuario,ip,port ); }
        else if (strcmp(command, "DISCONNECT") == 0) { resp = disconnect_user(usuario); }
        else if (strcmp(command, "PUBLISH") == 0) {resp = publish(usuario, filename, descripcion); }
        else if (strcmp(command, "LIST_USERS") == 0) {printf("en listusers"); resp = list_users(sc,usuario); }
        else{resp = -1;}

        char response[2];
        snprintf(response, sizeof(response), "%d", resp);
        response[1] = '\0';
        ret = sendMessage(sc, response, strlen(response));

        if (ret == -1) {
            printf("Error en envío respuesta desde el servidor\n");
            exit(-1);
        }
    }
    close(sc);
    pthread_exit(NULL);

}

int register_user(const char *username) {
    pthread_mutex_lock(&mutex_file);
    int result;

    if (exist(username, "reg_users.txt") == 1) {
        pthread_mutex_unlock(&mutex_file);
        return 1;
    }
    result = insert_value(username, "reg_users.txt");
    pthread_mutex_unlock(&mutex_file);
    return result;
}

int unregister_user(const char *username) {
    int user_found;
    pthread_mutex_lock(&mutex_file);
    if ((user_found = delete_user(username, "reg_users.txt")) == -1){return -1;}
    pthread_mutex_unlock(&mutex_file);
    return user_found ; 
}

int connect_user(const char *username, const char *ip, int port ) {
    int found;
    pthread_mutex_lock(&mutex_file);
    if (exist(username, "reg_users.txt")!=1){
        fprintf(stderr, "s> Usuario no registado, debes registrarte antes de conectarte\n");
        pthread_mutex_unlock(&mutex_file);
        return 1; //Usuario no existe
    }

    char record[1024];
    sprintf(record, "%s %s %d", username, ip, port);
    found = insert_value(record, "connected_usr.txt");
    pthread_mutex_unlock(&mutex_file);

    if (found == 0){return 0;} //Usuario añadido
    else if (found == 1){return 2;} //Usuario ya conectado
    else{return -1;}
}


int disconnect_user(const char *username) {
    pthread_mutex_lock(&mutex_file);
    int found = delete_user(username, "connected_usr.txt");
    pthread_mutex_unlock(&mutex_file);

    return found; 
}


int publish(const char *username, char filename[256], char descripcion[256]){
    pthread_mutex_lock(&mutex_file);
    int found = add_values(username, filename, descripcion);
    pthread_mutex_unlock(&mutex_file);
    if (found == -1){
        return -1;
    }
    return found;
}

int list_users(int sc, const char *username) {
    pthread_mutex_lock(&mutex_file);
    FILE *fp = fopen("connected_usr.txt", "r");
    
    char line[256];
    int found = 0;
    int count = 0;

    // Verificar si el usuario está conectado y contar los usuarios conectados
    while (fgets(line, sizeof(line), fp)) {
        char* token = strtok(line, " ");
        if (token && strcmp(token, username) == 0) {
            found = 1;
        }
        count++;
    }
    printf("Contador %d \n", count);

    if (!found) {
        fclose(fp);
        pthread_mutex_unlock(&mutex_file);
        printf("usuario no conectado");
        return 2; // Usuario no  conectado
    }

    // Reposicionar al inicio del archivo para enviar datos
    rewind(fp);
    char countStr[16];
    snprintf(countStr, sizeof(countStr), "%d", count);
    sendMessage(sc, countStr, strlen(countStr) + 1);

    // Enviar información de cada usuario conectado
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = 0; 
        strcat(line, "\0");
        printf("enviar info de usuarios");
        sendMessage(sc, line, strlen(line) + 1);
    }

    fclose(fp);
    pthread_mutex_unlock(&mutex_file);
    return 0; 
}

int main( int argc, char *argv[] ) {
    init();
    init_connections();
    int port = 0;
    int sd, sc;

    if (argc < 3) {
        fprintf(stderr, "Uso: %s -p <puerto>\n", argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0) {
            if (i + 1 < argc) { // Aseguramos que hay un argumento después de "-p"
                port = atoi(argv[i + 1]);
                i++; // Incrementamos 'i' para saltar el número de puerto
            } else {
                fprintf(stderr, "Error: se esperaba el número de puerto después de '-p'\n");
                return 1;
            }
        }
    }

    if (port == 0) {
        fprintf(stderr, "Número de puerto inválido o no especificado.\n");
        return 1;
    }

    printf("Servidor escuchando en el puerto %d\n", port);

    pthread_t thread;
    pthread_attr_t th_attr;
    pthread_mutex_init(&mutex_mensaje, NULL);
    pthread_cond_init(&cond_mensaje, NULL);
    pthread_attr_init(&th_attr);
    pthread_mutex_init(&mutex_file, NULL);
    pthread_cond_init(&cond_file, NULL);

    sd = serverSocket(INADDR_ANY, port, SOCK_STREAM);
    if (sd < 0) {
        printf ("SERVER: Error en serverSocket\n");
        return -1;
    }
    pthread_attr_setdetachstate(&th_attr, PTHREAD_CREATE_DETACHED);

    while (1) {
        sc = serverAccept(sd) ;
        if (sc < 0) {
            perror("Error en serverAccept");
            if (errno == EINTR) {
                continue; 
            }
            break; 
        }
        int *arg = malloc(sizeof(int));
        if (!arg) {
            printf("Error al asignar memoria\n");
            close(sc);
            continue;
        }
        *arg = sc;

        if (pthread_create(&thread, &th_attr, funcion_hilo,(void *) arg) != 0) {
            printf("Error al crear el hilo\n");
            free(arg);
            close(sc);
        }
        
    }

    close(sd);
    pthread_attr_destroy(&th_attr);
    pthread_mutex_destroy(&mutex_mensaje);
    pthread_mutex_destroy(&mutex_file);
    pthread_cond_destroy(&cond_mensaje);
    pthread_cond_destroy(&cond_file);
    return 0;
}