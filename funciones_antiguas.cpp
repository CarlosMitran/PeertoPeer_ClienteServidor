//
// Created by carlos on 30/04/24.
//

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
