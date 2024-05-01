//
// Created by carlos on 1/05/24.
//

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
    return 0; // Éxito
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
    FILE *fp;
    char buf[MAX_LINE_LEN];
    fp = fopen(file, "r");
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


int delete_user(const char *username, const char *file) {
    if ((exist(username, file)) == 0) {
        return 1;
    }
    int deleting = 0;
    char buf[MAX_LINE_LEN];
    FILE *fpold, *fpnew;
    fpold = fopen(file, "r");
    if (fpold == NULL) {
        return -1;
    }
    fpnew = fopen("temp.txt", "w");
    if (fpnew == NULL) {
        return -1;
    }
    while (fgets(buf, MAX_LINE_LEN, fpold) != NULL) {
        if (buf[0] == '@') {
            if (strcmp(buf +1, username) == 0){
                deleting = 1;
            }
            else{
                deleting = 0;
                fprintf(fpnew, "%s", buf);
            }
        }
        if (deleting == 0) {
            printf("%s", buf);
            fprintf(fpnew, "%s", buf);
        }
    }

    fclose(fpold);
    fclose(fpnew);
    remove(file);
    if (rename("temp.txt", file) != 0) {
        perror("Error renaming file \n");
        return -1;
    }
    return 0;
}

int insert_value(const char *username, const char *file){
    int exists;
    int found = 0;
    FILE *fp;
    exists = exist(username, file);
    if (exists == 1){
        return 1;
    }
    if (exists == -1){
        return -1;
    }
    fp = fopen(file, "r+");
    if (!fp) {
        return -1; // Error al abrir archivo
    }

    if (!found) {
        // Si no se encontró, añadir al final
        fseek(fp, 0, SEEK_END);
        fprintf(fp, "@%s\n", username);
    }
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