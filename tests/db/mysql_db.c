#include <stdio.h>
#include <stdbool.h>
#include "myutils.h"



int main(int argc, char **argv)
{
#ifdef _USE_MYSQL_
	MYSQL *conn = NULL;
	MYSQL_RES *res = NULL;

	my_init_mysql(&conn);
	my_set_con_option(conn);
	my_con_mysql(conn, "localhost", "root", "QwErTy1!", "test", 0, (CLIENT_COMPRESS | CLIENT_FOUND_ROWS));

	my_exec_query(conn, &res, "SELECT * FROM books", true);
	my_print_coninfo(res);
	my_print_res(res);
    
	my_exec_query(conn, NULL, "insert into books values('hello world', 1234, now(), false)", false);

	// 리소스 해제
	if (res != NULL)
		mysql_free_result(res);
	
	mysql_close(conn);

	return 0;
#endif
}
