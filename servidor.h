//
// Created by carlos on 26/04/24.
//

#ifndef FINAL_DISTRIBUIDOS_SERVIDOR_H
#define FINAL_DISTRIBUIDOS_SERVIDOR_H

int register_user(const char *usuario);
int connect_user(const char *username, const char *ip, int port);
int unregister_user(const char *username);
int disconnect_user(const char *username);
int publish(const char *username, char filename[256], char descripcion[256]);
int add_values(const char *username, char filename[256], char descripcion[256]);


#endif //FINAL_DISTRIBUIDOS_SERVIDOR_H
