#include "libserver.h"
#include "stdio.h"
#include "string.h"
#define MAX_LINE_LEN 256

int init() {
    // Verificar si el archivo existe y crearlo si es necesario
    FILE *fp = fopen("reg_users.txt", "a+");
    if (fp == NULL) {
        perror("Error al abrir el archivo de usuarios");
        return -1;
    }
    fclose(fp);
    return 0; 
}

int init_connections(){
    // Verificar si el archivo existe y crearlo si es necesario
    FILE *fp = fopen("connected_usr.txt", "w");
    if (fp == NULL) {
        perror("Error al abrir el archivo de usuarios");
        return -1;
    }
    fclose(fp);
    return 0; // Éxito
}


int exist(const char *usuario, const char *file) {
    FILE *fp = fopen(file, "r");
    if (!fp){
        return -1; //Error 
    }

    char line[1024];
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strstr(line, usuario) != NULL) {
            fclose(fp);
            return 1; // Usuario encontrado.
        }
    }
    fclose(fp);
    return 0; //Usuario no encontrado
}


int delete_user(const char *username, const char *file) {

    int found = 0;
    char buf[MAX_LINE_LEN];
    FILE *fpold, *fpnew;
    char expected_username[MAX_LINE_LEN];
    sprintf(expected_username, "@%s", username); 

    fpold = fopen(file, "r");
    if (fpold == NULL) {
        return -1;
    }
    fpnew = fopen("temp.txt", "w");
    if (fpnew == NULL) {
        fclose(fpold);
        return -1;
    }
    while (fgets(buf, MAX_LINE_LEN, fpold) != NULL) {
        buf[strcspn(buf, "\n")] = 0;
        if (strcmp(buf, expected_username) != 0) {
            fprintf(fpnew, "%s\n", buf);  // Escribir la línea como estaba
        } else {
            found = 1; // Se encontró el usuario y no se copia a temp
        }
    }

    fclose(fpold);
    fclose(fpnew);

   if (found) {
        if (remove(file) != 0 || rename("temp.txt", file) != 0) {
            perror("Error al actualizar el archivo de usuarios");
            return -1;
        }
    } else {
        // Eliminar el archivo temporal si no se encontró el usuario
        remove("temp.txt");
    }
    return found ? 0 : 1;

}

int insert_value(const char *username, const char *file){
    int exists;
    FILE *fp;
    fp = fopen(file, "r+");
    if (!fp) {
        fp = fopen(file, "w");
        if (!fp) {
            return 3; // Error al abrir archivo
        }
    }

    exists = exist(username, file);
    if (exists == 1){
        fclose(fp); 
        return 1; //Usuario ya esta en el archivo
    }
    else if (exists == -1){
        fclose(fp); 
        return -1; //Error 
    }
    // Si no se encontró, añadir al final
    fseek(fp, 0, SEEK_END);
    fprintf(fp, "@%s\n", username);
    fclose(fp);
    return 0;
}


int add_values(const char *username, char filename[256], char descripcion[256]){
    if ((exist(username, "reg_users.txt")) == 0){
        fprintf(stderr, "s> Usuario no encontrado\n");
        return 1;
    }

    FILE *fpold, *fpnew;
    char buf[256];
    fpold = fopen("reg_users.txt", "r");
    if (fpold == NULL) {
        return -1;
    }
    fpnew = fopen("temp.txt", "w");
    if (fpnew == NULL) {
        return -1;
    }
    while (fgets(buf, MAX_LINE_LEN, fpold) != NULL) {
        if (buf[0] == '@') {
            if (strcmp(buf+1, username) == 0) {
                printf("Found it\n");
                fprintf(fpnew, "%s\n", filename);
                fprintf(fpnew, "%s\n", descripcion);
            }
        }
        fprintf(fpnew, "%s", buf);
    }

    fclose(fpold);
    fclose(fpnew);
    remove("reg_users.txt");
    if (rename("temp.txt", "reg_users.txt") != 0) {
        perror("Error renaming file \n");
        return -1;
    }
    return 0;


}