//
// Created by carlos on 1/05/24.
//

#ifndef FINAL_DISTRIBUIDOS_LIBSERVER_H
#define FINAL_DISTRIBUIDOS_LIBSERVER_H

int init();
int init_connections();
int exist(const char *usuario, const char*file) ;
int delete_user(const char *username, const char *file);
int insert_value(const char *username, const char *file);
int add_publish_values(const char *username, char filename[256], char descripcion[256]);


#endif //FINAL_DISTRIBUIDOS_LIBSERVER_H
