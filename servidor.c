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
    int sc = *(int*)arg;
    free(arg);

    int ret;
    int resp;
    char buf[256];
    char peticion[256];
    char usuario[256];
    char filename[256];
    char descripcion[256];
    char command[256];

    //Leer comando
    
    if (readLine(sc, command, 256) < 0) {
        printf("s> Error en recepción de comando\n");
        close(sc);
        pthread_exit(NULL);
    }

    // Leer nombre usuario
    if (readLine(sc, usuario, sizeof(usuario)) < 0) {
        printf("s> Error en recepción de username\n");
        close(sc);
        pthread_exit(NULL);
    }

    printf("s> OPERACION FROM: %s\n", usuario);
    resp = init();

    if (strcmp(command, "REGISTER") == 0) {
        resp = register_user(usuario);
    } else {
        printf("s> Operacion desconocida\n");
        resp = -1;
    }
        
    ret = readLine(sc,filename , 256);
    if (ret < 0) {
        printf("Error en recepción op\n");
        pthread_exit(NULL);
    }

    ret = readLine(sc, descripcion, 256);
    if (ret < 0) {
        printf("Error en recepción op\n");
        pthread_exit(NULL);
    }
    char response[2];
    snprintf(response, sizeof(response), "%d", resp);
    sendMessage(sc, response, sizeof(response));
    close(sc);
    pthread_exit(NULL);

}

int init() {
    // Verificar si el archivo existe y crearlo si es necesario
    FILE *fp = fopen("reg_users.txt", "a+");
    if (fp == NULL) {
        perror("Error al abrir el archivo de usuarios");
        return -1;
    }
    fclose(fp);
    return 0; // Éxito
}

int register_user(const char *username) {
    FILE *fp;
    char line[256];
    int found = 0;

    pthread_mutex_lock(&mutex_file);
    fp = fopen("reg_users.txt", "r+");
    if (!fp) {
        pthread_mutex_unlock(&mutex_file);
        return -1; // Error al abrir archivo
    }

    // Buscar el usuario en el archivo
    while (fgets(line, sizeof(line), fp) != NULL) {
        line[strcspn(line, "\n")] = 0; 
        if (strcmp(line, username) == 0) {
            found = 1;
            break;
        }
    }

    if (!found) {
        printf("not found");
        // Si no se encontró, añadir al final
        fseek(fp, 0, SEEK_END);
        fprintf(fp, "%s\n", username);
    }

    fclose(fp);
    pthread_mutex_unlock(&mutex_file);
    return found ? 1 : 0; // 1 si el usuario ya existía, 0 si se añadió
}

int main( int argc, char *argv[] ) {
    
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
    pthread_attr_setdetachstate(&th_attr, PTHREAD_CREATE_DETACHED);

    sd = serverSocket(INADDR_ANY, port, SOCK_STREAM);
    if (sd < 0) {
        printf ("SERVER: Error en serverSocket\n");
        return -1;
    }

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