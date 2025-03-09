#ifndef _MYUTILS_H_
#define _MYUTILS_H_

#include "config.h"

#ifdef _USE_MYSQL_

#include "mysql/mysql.h"

void my_print_err_with_exit(MYSQL *conn, const char *where);
void my_init_mysql(MYSQL **conn);
void my_set_con_option(MYSQL *conn);
void my_con_mysql(MYSQL *conn, char *host, char *user, char *passwd, char *dbname, unsigned int port, unsigned long option);
void my_exec_query(MYSQL *conn, MYSQL_RES **res, const char *query, bool saveres);
void my_print_coninfo(MYSQL_RES *res);
void my_print_res(MYSQL_RES *res);

#endif

#endif