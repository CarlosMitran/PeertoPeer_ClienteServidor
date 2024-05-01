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
int mensaje_copiado = 0;
int file = 1;


void *funcion_hilo(void *arg) {
    int sc = *(int *) arg;
    free(arg);

    int ret;
    int resp;
    char usuario[256];
    char filename[256];
    char descripcion[256];
    char command[256];

    //Leer comando
    while(1){
    ret = readLine(sc, command, sizeof(command));
    if (ret <= 0) {
        printf("s> Error en recepción de comando o comando no soportado\n");
        //pthread_exit(NULL);
    }

    init();

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
    else if (strcmp(command, "CONNECT") == 0) { resp = connect_user(usuario); }
    else if (strcmp(command, "DISCONNECT") == 0) { resp = disconnect_user(usuario); }
    else if (strcmp(command, "PUBLISH") == 0) {resp = publish(usuario, filename, descripcion); }
    else{resp = -1;}

    char response[2];
    snprintf(response, sizeof(response), "%d", resp);
    ret = sendMessage(sc, response, 2);

    if (ret == -1) {
        printf("Error en envío respuesta desdf el servidor\n");
        exit(-1);
    }
    }

    close(sc);
    pthread_exit(NULL);

}



int register_user(const char *username) {
    pthread_mutex_lock(&mutex_file);
    int found = insert_value(username, "reg_users.txt");
    pthread_mutex_unlock(&mutex_file);
    return found ; // 1 si el usuario ya existía, 0 si se añadió
}

int unregister_user(const char *username) {
    int user_found;
    pthread_mutex_lock(&mutex_file);
    if ((user_found = delete_user(username, "reg_users.txt")) == -1){return -1;}
    pthread_mutex_unlock(&mutex_file);
    return user_found ? 0 : 1;  // Devuelve 0 si el usuario fue eliminado, 1 si no se encontró
}

int connect_user(const char *username) {
    int is_connected;
    int found;
    pthread_mutex_lock(&mutex_file);
    if (exist(username, "reg_users.txt")!=1){
        fprintf(stderr, "s> Usuario no registado, debes registrarte antes de conectarte\n");
        return -1;
    }
    found = insert_value(username, "connected_usr.txt");
    if (found == 1){is_connected = 2;}
    else if (found == -1){return -1;}
    else{is_connected = 0;}

    pthread_mutex_unlock(&mutex_file);

    return is_connected ? 2 : 0; // Devuelve 2 si ya estaba conectado, 0 si se conectó ahora
}


int disconnect_user(const char *username) {


    pthread_mutex_lock(&mutex_file);

    int found = delete_user(username, "connected_usr.txt");

    pthread_mutex_unlock(&mutex_file);

    return found ? 0 : 1; 
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


int main( int argc, char *argv[] ) {
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
