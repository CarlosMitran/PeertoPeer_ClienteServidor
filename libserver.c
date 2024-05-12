#include "libserver.h"
#include "stdio.h"
#include "string.h"
#include "rpc_funciones.h"
#define MAX_LINE_LEN 256

int init() {
    // Verificar si el archivo existe y crearlo si es necesario, no modifica el archivo si ya existe
    FILE *fp = fopen("reg_users.txt", "a+");
    if (fp == NULL) {
        perror("Error al abrir el archivo de usuarios");
        return -1;
    }
    fclose(fp);
    return 0; 
}

int init_connections(){
    // Verificar si el archivo existe y crearlo si es necesario, reinicia la información de los usuarios conectados
    FILE *fp = fopen("connected_usr.txt", "w");
    if (fp == NULL) {
        perror("Error al abrir el archivo de usuarios");
        return -1;
    }
    fclose(fp);
    return 0; // Éxito
}



int exist(const char *usuario, const char *file) {
    //Función para encontrar si un usuario, es decir, string que empieza por @, existe en el file que buscamos.
    FILE *fp;
    //Temp_buf elimina el @ y cualquier espacio que pueda haber para comparar strings con el usuario
    char buf[MAX_LINE_LEN];
    char temp_buf[MAX_LINE_LEN];
    fp = fopen(file, "r");
    if (fp == NULL) {
        fclose(fp);
        return -1;
    }
    //Se busca en todas las líneas del archivo, si se encuentra @ comparamos para ver si hemos encontrado el usuario
    while (fgets(buf, MAX_LINE_LEN, fp) != NULL) {
        if (buf[0] == '@') {
            int n = strcspn(buf, "\n");
            strncpy(temp_buf, buf, n);
            if (strcmp(temp_buf + 1, usuario) == 0) {
                fclose(fp);
                //Usuario encontrado
                return 1;
            }
            memset(temp_buf, 0, MAX_LINE_LEN);
        }
    }
    //Usuario no encontrado
    fclose(fp);
    return 0;
}


int delete_user(const char *username,const char *file) {
    //Función para eliminar usuarios de los files
    if ((exist(username, file)) != 1) {
        //El usuario no existe o ha ocurrido un error
        return 1;
    }
    int counter = 0;
    //Temp_buf realiza strcmp sin el @
    char buf[MAX_LINE_LEN];
    char temp_buf[MAX_LINE_LEN];
    //Creamos un file para reescribir los datos sin el usuario que queremos borrar
    FILE *fpold, *fpnew;
    fpold = fopen(file, "r");
    if (fpold == NULL) {
        return 2;
    }
    fpnew = fopen("temp.txt", "w");
    if (fpnew == NULL) {
        return 2;
    }
    //Reescribimos todos los datos, si nos encontramos con un usuario, no escribimos sus datos.
    while (fgets(buf, MAX_LINE_LEN, fpold) != NULL) {
        if (counter <= 0) {
            if (buf[0] == '@') {
                int n = strcspn(buf, "\n");
                strncpy(temp_buf, buf, n);
                if (strcmp(temp_buf+1, username) == 0){
                    counter = 2;
                    continue;
                }
                memset(temp_buf, 0, MAX_LINE_LEN);
            }
            fprintf(fpnew, "%s", buf);
        }
        else{
            counter = counter-1;
            if (buf[0] == '@'){
                counter = 0;
                fprintf(fpnew, "%s", buf);
            }
        }
    }
    fclose(fpold);
    fclose(fpnew);
    if (rename("temp.txt", file) != 0) {
        perror("Error renaming file \n");
        return 2;
    }
    return 0;
}

int insert_value(const char *username, const char *file){
    //Insertar valores de usuario a un file.
    int exists;
    FILE *fp;
    fp = fopen(file, "r+");
    if (!fp) {
        fp = fopen(file, "w");
        if (!fp) {
            return 2; // Error al abrir archivo
        }
    }
    //Si el usuario ya existe no lo añadimos
    exists = exist(username, file);
    if (exists == 1){
        fclose(fp); 
        return 1; //Usuario ya esta en el archivo
    } else if (exists == -1){
        fclose(fp); 
        return 2; //Error
    }
    // Si no se encontró, añadir al final
    fseek(fp, 0, SEEK_END);
    fprintf(fp, "%s\n", username);
    fclose(fp);
    return 0;
}


int add_publish_values(const char *username, char filename[256], char descripcion[256]){
    //Añadir valores de publicación
    int found = 1;
    //Si el usuario no existe añadiremos también el username al fichero, si no solo añadimos el filename y la descripción
    //debajo del usuario.
    if ((exist(username, "published_content.txt")) == 0){
        found = 0;
    }
    //Creamos el archivo nuevo en el que se va a reescribir
    FILE *fpold, *fpnew;
    char temp_buf[256];
    char buf[256];
    fpold = fopen("published_content.txt", "r");
    if (fpold == NULL) {
        return -1;
    }
    fpnew = fopen("temp.txt", "w");
    if (fpnew == NULL) {
        return -1;
    }
    //Cuando encontramos el usuario, escribimos ambas líneas debajo de este
    while (fgets(buf, MAX_LINE_LEN, fpold) != NULL) {
        if (buf[0] == '@') {
            int n = strcspn(buf, "\n");
            strncpy(temp_buf, buf, n);
            if (strcmp(temp_buf+1, username) == 0) {
                fprintf(fpnew, "%s", buf);
                printf("Found it\n");
                fprintf(fpnew, "%s\n", filename);
                fprintf(fpnew, "%s\n\n", descripcion);
                memset(temp_buf, 0, MAX_LINE_LEN);
                continue;
            }
            memset(temp_buf, 0, MAX_LINE_LEN);
        }
        fprintf(fpnew, "%s", buf);
    }
    //Si el usuario no se encuentra, se escribe al final del todo la información del publish
    if (found == 0){
        fprintf(fpnew, "@%s\n", username);
        fprintf(fpnew, "%s\n", filename);
        fprintf(fpnew, "%s\n\n", descripcion);
    }

    fclose(fpold);
    fclose(fpnew);
    remove("published_content.txt");
    if (rename("temp.txt", "published_content.txt") != 0) {
        perror("Error renaming file \n");
        return -1;
    }
    return 0;
}



int callPrintService(char *stringToPrint)
{
    //Llamada al servicio RPC, damos el resultado a imprimir (usuario, operacion, fecha)
    int result;
    result = printService_1(stringToPrint);
    return result;
}