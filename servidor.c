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
    ret = readLine(sc, command, 256);
    if (ret < 0 || strcmp(command, "REGISTER") != 0) {
        printf("s> Error en recepción de comando o comando no soportado\n");
        close(sc);
        pthread_exit(NULL);
    } 

    //Leer nombre usuario
    ret = readLine(sc, usuario, sizeof(usuario));
    if (ret < 0) {
        printf("s> Error en recepción de username\n");
        close(sc);
        pthread_exit(NULL);
    }

    //strcpy(peticion, buf);
    printf("s> OPERACION FROM: %s\n", usuario);
    resp = init();

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

int set_value(int key, char *value1, int N_value2, double *V_value_2) {
    //Escribir el set de valores en el archivo .txt
    if (exist(key)){
        perror("La clave ya ha sido utilizada \n");
        return -1;
    }
    FILE *fp;
    fp = fopen("Claves.txt", "a");
    if (fp == NULL) {
        return -1;
    }
    fprintf(fp, "@%d\n", key);
    fprintf(fp, "%s\n", value1);
    fprintf(fp, "%d\n", N_value2);
    for (int i = 0; i < N_value2 - 1; ++i) {
        fprintf(fp, "%lf,", V_value_2[i]);
    }
    fprintf(fp, "%lf\n", V_value_2[N_value2 - 1]);
    fclose(fp);
    return 0;
}

int get_value(int key, char *value1, int *N_value2, double *V_value2) {
    FILE *fp;
    char buf[MAX_LINE_LEN];
    fp = fopen("Claves.txt", "r");
    if (fp == NULL) {
        return -1;
    }
    while (fgets(buf, MAX_LINE_LEN, fp) != NULL) {
        if (buf[0] == '@') {
            int keytry = atoi(buf + 1);
            if (keytry == key) {
                fgets(buf, MAX_LINE_LEN, fp);
                strcpy(value1, buf);
                fgets(buf, MAX_LINE_LEN, fp);
                *N_value2 = atoi(buf);
                fgets(buf, MAX_LINE_LEN, fp);
                char *token = strtok(buf, ",");
                int i = 0;
                while (token != NULL) {
                    V_value2[i] = strtod(token, NULL);
                    token = strtok(NULL, ",");
                    i++;
                }
                fclose(fp);
                return 0;
            }
        }
    }
    fclose(fp);
    return -1;
}

int modify_value(int key, char *value1, int N_value2, double *V_value2) {
    if ((exist(key)) != 1) {
        return -1;
    }
    FILE *fpold, *fpnew;
    char buf[MAX_LINE_LEN];
    fpold = fopen("Claves.txt", "r");
    if (fpold == NULL) {
        return -1;
    }
    fpnew = fopen("temp.txt", "w");
    if (fpnew == NULL) {
        return -1;
    }
    while (fgets(buf, MAX_LINE_LEN, fpold) != NULL) {
        if (buf[0] == '@') {
            int keytry = atoi(buf + 1);
            if (keytry == key) {
                fprintf(fpnew, "@%d\n", key);
                fgets(buf, MAX_LINE_LEN, fpold);
                fprintf(fpnew, "%s\n", value1);
                fgets(buf, MAX_LINE_LEN, fpold);
                fprintf(fpnew, "%d\n", N_value2);
                fgets(buf, MAX_LINE_LEN, fpold);
                for (int i = 0; i < N_value2 - 1; ++i) {
                    fprintf(fpnew, "%lf,", V_value2[i]);
                }
                fprintf(fpnew, "%lf\n", V_value2[N_value2 - 1]);
                //fgets(buf, MAX_LINE_LEN, fpold);
                continue;
            }
        }
        fprintf(fpnew, "%s", buf);
    }
    fclose(fpold);
    fclose(fpnew);
    if (rename("temp.txt", "Claves.txt") != 0) {
        perror("Error renaming file \n");
        return 1;
    }
    return 0;
}

int delete_key(int key) {
    if ((exist(key)) != 1) {
        return -1;
    }
    int counter = 0;
    char buf[MAX_LINE_LEN];
    FILE *fpold, *fpnew;
    fpold = fopen("Claves.txt", "r");
    if (fpold == NULL) {
        return -1;
    }
    fpnew = fopen("temp.txt", "w");
    if (fpnew == NULL) {
        return -1;
    }
    while (fgets(buf, MAX_LINE_LEN, fpold) != NULL) {
        if (counter <= 0) {
            if (buf[0] == '@') {
                int keytry = atoi(buf + 1);
                if (keytry == key) {
                    counter = 3;
                    continue;
                }
            }
            printf("%s", buf);
            fprintf(fpnew, "%s", buf);

        }

    }
    fclose(fpold);
    fclose(fpnew);
    if (rename("temp.txt", "Claves.txt") != 0) {
        perror("Error renaming file \n");
        return 1;
    }
    return 0;
}

int exist(int key) {
    FILE *fp;
    char buf[MAX_LINE_LEN];
    fp = fopen("Claves.txt", "r");
    if (fp == NULL) {
        fclose(fp);
        return -1;
    }
    while (fgets(buf, MAX_LINE_LEN, fp) != NULL) {
        if (buf[0] == '@') {
            int keytry = atoi(buf + 1);
            if (keytry == key) {
                fclose(fp);
                return 1;
            }
        }
    }
    fclose(fp);
    return 0;

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