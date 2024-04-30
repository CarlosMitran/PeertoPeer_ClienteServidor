//
// Created by carlos on 26/04/24.
//

#ifndef FINAL_DISTRIBUIDOS_SERVIDOR_H
#define FINAL_DISTRIBUIDOS_SERVIDOR_H
int init();
int set_value(int key, char *value1, int N_value2, double *V_value_2);
int get_value(int key, char *value1, int *N_value2, double *V_value2);
int modify_value(int key, char *value1, int N_value2, double *V_value2);
int delete_key(int key);
int exist(const char *usuario);
int register_user(const char *usuario);
int connect_user(char *usuario);


#endif //FINAL_DISTRIBUIDOS_SERVIDOR_H
