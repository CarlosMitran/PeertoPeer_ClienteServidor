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

    while (1) {
        //Leer comando
        ret = readLine(sc, command, sizeof(command));
        if (ret <= 0 ) {
            printf("s> Error en recepción de comando o comando no soportado\n");
            //pthread_exit(NULL);
        }
        
        resp = init();
        // Leer nombre usuario
        if (readLine(sc, usuario, sizeof(usuario)) < 0) {
            printf("s> Error en recepción de username\n");
            close(sc);
            pthread_exit(NULL);
        }
        printf("s> OPERACION FROM: %s\n", usuario);
        
        
        if (strcmp(command, "REGISTER") == 0)
        {
            resp = register_user(usuario);
        }
        else if (strcmp(command, "UNREGISTER") == 0){
            resp = unregister_user(usuario);
        }
         else if (strcmp(command, "CONNECT") == 0){
            resp = connect_user(usuario);
        }
        else if (strcmp(command, "DISCONNECT") == 0){
            resp = unregister_user(usuario);
        }
        char response[2];
        snprintf(response, sizeof(response), "%d", resp);
        response[1] = '\0';
        ret = sendMessage(sc, response, 2);
        if (ret == -1) {
            printf("Error en envío respuesta desdf el servidor\n");
            exit(-1) ;
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
        
    }

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
    int found = 0;

    pthread_mutex_lock(&mutex_file);
    int exists;
    exists = exist(username);
    if (exists == 1){
        found = 1;
    }
    if (exists == -1){
        return -1;
    }

    fp = fopen("reg_users.txt", "r+");
    if (!fp) {
        pthread_mutex_unlock(&mutex_file);
        return -1; // Error al abrir archivo
    }

    if (!found) {
        // Si no se encontró, añadir al final
        fseek(fp, 0, SEEK_END);
        fprintf(fp, "@%s\n", username);
    }

    fclose(fp);
    pthread_mutex_unlock(&mutex_file);
    return found ; // 1 si el usuario ya existía, 0 si se añadió
}

int unregister_user(const char *username) {
    FILE *fp, *fp_temp;
    char buf[MAX_LINE_LEN];
    int user_found = 0;

    pthread_mutex_lock(&mutex_file);

    fp = fopen("reg_users.txt", "r");
    if (!fp) {
        pthread_mutex_unlock(&mutex_file);
        return -1;  
    }

    fp_temp = fopen("reg_users.tmp", "w");
    if (!fp_temp) {
        fclose(fp);
        pthread_mutex_unlock(&mutex_file);
        return -1; 
    }

    // Leer el archivo original y escribir todo excepto el usuario a eliminar en el archivo temporal
    while (fgets(buf, MAX_LINE_LEN, fp) != NULL) {
        buf[strcspn(buf, "\n")] = 0;
        
        if (buf[0] == '@') {
            if (strcmp(buf + 1, username) == 0) {
                user_found = 1;
                continue;  
            }
        }
        fprintf(fp_temp, "%s\n", buf);  
    }

    fclose(fp);
    fclose(fp_temp);

    // Eliminar el archivo original y renombrar el temporal
    if (user_found) {
        remove("reg_users.txt");
        rename("reg_users.tmp", "reg_users.txt");
    } else {
        remove("reg_users.tmp");  // Si no se encontró el usuario, eliminar el archivo temporal
    }

    pthread_mutex_unlock(&mutex_file);
    return user_found ? 0 : 1;  // Devuelve 0 si el usuario fue eliminado, 1 si no se encontró
}

int connect_user(const char *username) {
    FILE *fp_reg, *fp_conn;
    int is_registered = 0;
    int is_connected = 0;

    pthread_mutex_lock(&mutex_file);

    // Primero, verificamos si el usuario está registrado.
    is_registered = exist(username);
    if (is_registered != 1) {
        pthread_mutex_unlock(&mutex_file);
        return 1; // Usuario no registrado
    }

    // Abrimos el archivo de usuarios conectados para verificar si ya está conectado
    fp_conn = fopen("connected_usr.txt", "r+");
    if (!fp_conn) {
        // Si no se puede abrir el archivo, intentamos crearlo.
        fp_conn = fopen("connected_usr.txt", "w+");
        if (!fp_conn) {
            pthread_mutex_unlock(&mutex_file);
            return -1; // Error al abrir o crear archivo
        }
    }

    // Buscar si el usuario ya está conectado
    char buf[MAX_LINE_LEN];
    while (fgets(buf, MAX_LINE_LEN, fp_conn) != NULL) {
        buf[strcspn(buf, "\n")] = 0; // Elimina el salto de línea
        if (buf[0] == '@' && strcmp(buf + 1, username) == 0) {
            is_connected = 1;
            break;
        }
    }

    // Si el usuario no está ya conectado, lo añadimos.
    if (!is_connected) {
        fseek(fp_conn, 0, SEEK_END);
        fprintf(fp_conn, "@%s\n", username);
    }

    // Cerrar archivo de usuarios conectados
    fclose(fp_conn);
    pthread_mutex_unlock(&mutex_file);

    return is_connected ? 2 : 0; // Devuelve 2 si ya estaba conectado, 0 si se conectó ahora
}

int disconnect_user(const char *username) {
    FILE *fp, *fp_temp;
    char buf[MAX_LINE_LEN];
    int found = 0;

    pthread_mutex_lock(&mutex_file);

    // Abrir el archivo de usuarios conectados y un archivo temporal
    fp = fopen("connected_usr.txt", "r");
    fp_temp = fopen("temp_connected_usr.txt", "w");

    if (!fp || !fp_temp) {
        if (fp) fclose(fp);
        if (fp_temp) fclose(fp_temp);
        pthread_mutex_unlock(&mutex_file);
        return -1; 
    }

    while (fgets(buf, sizeof(buf), fp) != NULL) {
        buf[strcspn(buf, "\n")] = 0; 
        if (buf[0] == '@' && strcmp(buf + 1, username) == 0) {
            found = 1; 
        } else {
            fprintf(fp_temp, "%s\n", buf); 
        }
    }

    fclose(fp);
    fclose(fp_temp);

    // Borrar archivo original y renombrar el temporal
    remove("connected_usr.txt");
    rename("temp_connected_usr.txt", "connected_usr.txt");

    pthread_mutex_unlock(&mutex_file);

    return found ? 0 : 1; 
}


int exist(const char *usuario) {
    FILE *fp;
    char buf[MAX_LINE_LEN];
    fp = fopen("reg_users.txt", "r");
    if (fp == NULL) {
        fclose(fp);
        return -1;
    }
    while (fgets(buf, MAX_LINE_LEN, fp) != NULL) {
        buf[strcspn(buf, "\n")] = 0;
        if (buf[0] == '@') {
            if (strcmp(buf + 1, usuario) == 0) {
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