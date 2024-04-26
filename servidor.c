#include <mqueue.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "claves.h"
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "comm.h"

#define MAX_LINE_LEN 255
#define MAX_THREADS    10
#define MAX_PETICIONES    256
#define ERROR 0

struct peticion buffer_peticiones[MAX_PETICIONES];
mqd_t q_servidor;
pthread_mutex_t mutex_mensaje;
pthread_mutex_t mutex_file;
pthread_cond_t cond_mensaje;
pthread_cond_t cond_file;
int mensaje_copiado = 0;
int file = 1;

//Informacion que el hilo necesita para procesar la peticion
struct inf_hilo {
    struct peticion mess;
};

void *funcion_hilo(void *arg) {
    pthread_mutex_lock(&mutex_mensaje);
    //args->mess Accedo a mess de la estructura inf_hilo y lo guardo en mess, de tipo struct peticion
    int sc = *(int*)arg;
    struct peticion mess;
    int ret;
    ret = recvMessage(sc, (char *) &(mess.op), sizeof(int));
    if (ret < 0) {
        printf("Error en recepción op\n");
        exit(-1) ;
    }
    ret = recvMessage(sc, (char *) &mess.key, sizeof(int));
    if (ret < 0) {
        printf("Error en recepción op\n");
        exit(-1) ;
    }
    ret = recvMessage(sc, (char *) &mess.value1, sizeof(char)*256);
    if (ret < 0) {
        printf("Error en recepción op\n");
        exit(-1) ;
    }
    ret = recvMessage(sc, (char *) &mess.value2, sizeof(int));
    if (ret < 0) {
        printf("Error en recepción op\n");
        exit(-1) ;
    }
    ret = recvMessage(sc, (char *) &mess.V_value2, sizeof(double )*32);
    if (ret < 0) {
        printf("Error en recepción op\n");
        exit(-1) ;
    }
    mensaje_copiado = 1;
    pthread_cond_signal(&cond_mensaje);
    pthread_mutex_unlock(&mutex_mensaje);

    struct respuesta resp;
    pthread_mutex_lock(&mutex_file);
    while(file == 0) {
        pthread_cond_wait(&cond_file, &mutex_file);
    }
    if (mess.op == 0) {
        resp.res = init();
    }
    //set_value
    else if (mess.op == 1) {
        resp.res = set_value(mess.key, mess.value1, mess.value2, mess.V_value2);
    }
    //get_value
    else if (mess.op == 2) {
        resp.res = get_value(mess.key, resp.value1, &resp.value2, resp.V_value2);
    }
    //modify_value
    else if (mess.op == 3) {
        resp.res = modify_value(mess.key, mess.value1, mess.value2, mess.V_value2);
    }
    //delete_key
    else if (mess.op == 4) {
        resp.res = delete_key(mess.key);
    }
    //exist
    else if (mess.op == 5) {
        resp.res = exist(mess.key);
    }
    //operacion desconocida
    else {
        perror("Operacion desconocida");
    }
    file = 1;
    pthread_cond_signal(&cond_file);
    pthread_mutex_unlock(&mutex_file);

    //enviamos información a cliente
    ret = sendMessage(sc, (char *)&resp.res, sizeof(int));
    if (ret == -1) {
        printf("Error en envío res\n");
        exit(-1) ;
    }
    ret = sendMessage(sc, (char *)&resp.key, sizeof(int));
    if (ret == -1) {
        printf("Error en envío key\n");
        exit(-1) ;
    }
    ret = sendMessage(sc, (char *)&resp.value1, sizeof(char)*256);
    if (ret == -1) {
        printf("Error en envío value1\n");
        exit(-1) ;
    }
    ret = sendMessage(sc, (char *)&resp.value2, sizeof(int));
    if (ret == -1) {
        printf("Error en envío value2\n");
        exit(-1) ;
    }
    ret = sendMessage(sc, (char *)&resp.V_value2, sizeof(double )*32);
    if (ret == -1) {
        printf("Error en envío V_value2\n");
        exit(-1) ;
    }


    pthread_exit(0);
}

int init() {
    //Inicializa .txt, si ya está creada la borra y la vuelve a crear, si no la vacía
    FILE *fp;
    fp = fopen("Claves.txt", "w");
    if (fp == NULL) {
        return -1;
    }
    fclose(fp);
    return 0;
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
    pthread_t thread;
    pthread_attr_t th_attr;

    if (argc < 2) {
        printf("Uso: %s <puerto>\n", argv[0]);
        exit(-1) ;
    }

    pthread_mutex_init(&mutex_mensaje, NULL);
    pthread_cond_init(&cond_mensaje, NULL);
    pthread_attr_init(&th_attr);
    pthread_mutex_init(&mutex_file, NULL);
    pthread_cond_init(&cond_file, NULL);

    int sd, sc;

    sd = serverSocket(INADDR_ANY, atoi(argv[1]), SOCK_STREAM);
    if (sd < 0) {
        printf ("SERVER: Error en serverSocket\n");
        return 0;
    }

    pthread_attr_setdetachstate(&th_attr, PTHREAD_CREATE_DETACHED);
    while (1) {
        sc = serverAccept(sd) ;
        if (sc < 0) {
            printf("Error en serverAccept\n");
            continue ;
        }
        int *arg = malloc(sizeof(int));
        *arg = sc;
        int error = pthread_create(&thread, &th_attr, funcion_hilo,(void *) arg);
        if (error == 0) {
            pthread_mutex_lock(&mutex_mensaje);
            mensaje_copiado = 0;
            while (mensaje_copiado == 0) {
                pthread_cond_wait(&cond_mensaje, &mutex_mensaje);
                mensaje_copiado = 1;
                pthread_mutex_unlock(&mutex_mensaje);
            }
            
        }
        free(arg);
    }
   
    close (sd);
    return 0;

}
