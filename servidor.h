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
int list_users(int sc, const char *username);
int delete_file(const char *username, const char *filename);
int list_content(int sc, const char *username, const char *userpeticion);
int quit(int sc, const char *username);
#endif //FINAL_DISTRIBUIDOS_SERVIDOR_H
