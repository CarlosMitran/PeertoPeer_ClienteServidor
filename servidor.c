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
        if (readLine(sc, command, sizeof(command))<= 0) {
            close(sc);
            pthread_exit(NULL);
            //break;
        }
        size_t length = strlen(command);
        if (length > 0 && command[length - 1] == '\n') {
            command[length - 1] = '\0';
        }
        printf("s> Command received: '%s'\n", command);
        
        
        // Leer nombre usuario
        if (readLine(sc, usuario, sizeof(usuario)) < 0) {
            printf("s> Error en recepción de username\n");
            close(sc);
            pthread_exit(NULL);
        }
        printf("s> OPERACION FROM: %s\n", usuario);

        ret = readLine(sc, filename, 256);
        if (ret < 0) {
            printf("Error en recepción filename\n");
            pthread_exit(NULL);
            close(sc);
        }

        ret = readLine(sc, descripcion, 256);
        if (ret < 0) {
            printf("Error en recepción descripcion\n");
            pthread_exit(NULL);
            close(sc);
        }
        if (strcmp(command, "REGISTER") == 0) {resp = register_user(usuario); }
        else if (strcmp(command, "UNREGISTER") == 0) { resp = unregister_user(usuario); }
        else if (strcmp(command, "CONNECT") == 0) { resp = connect_user(usuario,ip,port ); }
        else if (strcmp(command, "DISCONNECT") == 0) { resp = disconnect_user(usuario); }
        else if (strcmp(command, "PUBLISH") == 0) {resp = publish(usuario, filename, descripcion); }
        else if (strcmp(command, "LIST_USERS") == 0) {resp = list_users(sc,usuario); }
        else if(strcmp(command, "DELETE") == 0){resp = delete_file(usuario, filename);}
        else if(strcmp(command, "LIST_CONTENT") == 0){resp = list_content(sc, filename);}
        else if(strcmp(command, "QUIT") == 0) {
            resp = quit(sc,usuario);
            

        }
        else{printf("respeusta -1");resp = -1;}

        char response[3];
        snprintf(response, sizeof(response), "%d", resp);
        response[2] = '\0';
        ret = sendMessage(sc, response, strlen(response));

        if (ret == -1) {
            printf("Error en envío respuesta desde el servidor\n");
            exit(-1);
            close(sc);
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
    char usernames[256];
    sprintf(usernames, "@%s", username);
    result = insert_value(usernames, "reg_users.txt");
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
    if (exist(username, "connected_usr.txt") == 1){
        fprintf(stderr, "s> Usuario ya conectado\n");
        pthread_mutex_unlock(&mutex_file);
        return 2; //Usuario no existe
    }
    char usernames[256];
    sprintf(usernames, "@%s", username );
    found = insert_value(usernames, "connected_usr.txt");

    insert_value(ip, "connected_usr.txt");
    char ports[256];
    sprintf(ports, "%d", port);
    insert_value(ports, "connected_usr.txt");

    pthread_mutex_unlock(&mutex_file);
    if (found == 0){return 0;} //Usuario añadido
    else if (found == 1){return 2;} //Usuario ya conectado
    else{return -1;}
}

int disconnect_user(const char *username) {
    pthread_mutex_lock(&mutex_file);
    int found = delete_user(username, "connected_usr.txt");
    printf("disconnect response is %d\n", found);
    pthread_mutex_unlock(&mutex_file);
    return found; 
}

int publish(const char *username, char filename[256], char descripcion[256]){
    pthread_mutex_lock(&mutex_file);
    int found = add_publish_values(username, filename, descripcion);
    pthread_mutex_unlock(&mutex_file);
    if (found == -1){
        return -1;
    }
    return found;
}

int list_users(int sc, const char *username) {
    pthread_mutex_lock(&mutex_file);
    FILE *fp = fopen("connected_usr.txt", "r");

    char buf[256];
    char temp_buf[265];
    int counter = 0;
    int typecounter = 1;

    // Enviar información de cada usuario conectado
    while (fgets(buf, MAX_LINE_LEN, fp)) {
        if (counter <= 0) {
            if (buf[0] == '@') {
                int n = strcspn(buf, "\n");
                strncpy(temp_buf, buf, n);
                if (strcmp(temp_buf+1, username) == 0){
                    counter = 2;
                    memset(temp_buf, 0, MAX_LINE_LEN);
                    continue;
                }
            }
        }
        else{counter = counter-1; continue;}
        if (typecounter == 1){sprintf(temp_buf, "%s", buf);}
        if (typecounter == 2){sprintf(temp_buf, "@IP%s",buf);}
        if (typecounter == 3){sprintf(temp_buf, "@PORT%s", buf); typecounter = 0;}
        sendMessage(sc, temp_buf, strlen(temp_buf));

        memset(temp_buf, 0, MAX_LINE_LEN);

        typecounter = typecounter + 1;
    }
    fclose(fp);
    pthread_mutex_unlock(&mutex_file);
    return 0; 
}

int delete_file(const char *username, const char *filename){
    pthread_mutex_lock(&mutex_file);
    char buf[256];
    char temp_buf[256];
    int found = 0;
    int counter = 0;
    FILE *fpold, *fpnew;
    fpold = fopen("published_content.txt", "r");
    if (fpold == NULL) {
        pthread_mutex_unlock(&mutex_file);
        return 4;
    }
    fpnew = fopen("temp.txt", "w");
    if (fpnew == NULL) {
        pthread_mutex_unlock(&mutex_file);
        return 4;
    }
    while (fgets(buf, MAX_LINE_LEN, fpold) != NULL) {
        int n = strcspn(buf, "\n");
        strncpy(temp_buf, buf, n);
        if (strcmp(temp_buf, filename) == 0) {
            found = 1;
            counter = 1;
            memset(temp_buf, 0, MAX_LINE_LEN);
            continue;
        }
        if (counter <= 0){
            memset(temp_buf, 0, MAX_LINE_LEN);
            printf("%s", buf);
            fprintf(fpnew, "%s", buf);
        }
        counter = counter -1;
    }

    fclose(fpold);
    fclose(fpnew);
    if (rename("temp.txt", "published_content.txt") != 0) {
        perror("Error renaming file \n");
        pthread_mutex_unlock(&mutex_file);
        return 4;
    }
    if (found != 1){
        pthread_mutex_unlock(&mutex_file);
        return 3;
    }
    pthread_mutex_unlock(&mutex_file);
    return 0;

}

int list_content(int sc, const char *username){
    pthread_mutex_lock(&mutex_file);
    FILE *fp = fopen("published_content.txt", "r");

    char buf[256];
    char temp_buf[265];


    while (fgets(buf, MAX_LINE_LEN, fp)!= NULL) {
        if (buf[0] == '@') {
            int n = strcspn(buf, "\n");
            strncpy(temp_buf, buf, n);
            if (strcmp(temp_buf+1, username) == 0){
                memset(temp_buf, 0, MAX_LINE_LEN);
                while(fgets(buf, MAX_LINE_LEN, fp)!=NULL){
                    if(buf[0] == '@'){break;}
                    sprintf(temp_buf, "@%s", buf);
                    sendMessage(sc, temp_buf, strlen(temp_buf));
                    memset(temp_buf, 0, MAX_LINE_LEN);
                }
                break;
            }
            memset(temp_buf, 0, MAX_LINE_LEN);
        }
    }
    fclose(fp);
    pthread_mutex_unlock(&mutex_file);
    return 0;

}

int quit(int sc, const char *username){
    pthread_mutex_lock(&mutex_file);
    int found = delete_user(username, "connected_usr.txt");
    pthread_mutex_unlock(&mutex_file);
    if (found==0) {
        return 0;
    }
    return 1;
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