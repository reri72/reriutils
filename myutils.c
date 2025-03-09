#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "myutils.h"

void my_print_err_with_exit(MYSQL *conn, const char *where)
{
    fprintf(stderr, "exit from [%s]\n", where);
	if (conn != NULL)
		mysql_close(conn);

    exit(1);
}

void my_init_mysql(MYSQL **conn)
{
	*conn = mysql_init(NULL);
	if (conn == NULL)
		my_print_err_with_exit(NULL, __FUNCTION__);
}

void my_set_con_option(MYSQL *conn)
{
	mysql_options(conn, MYSQL_SET_CHARSET_NAME, "utf8mb4");
}

void my_con_mysql(MYSQL *conn, char *host, char *user, char *passwd, char *dbname, unsigned int port, unsigned long option)
{
	if ( mysql_real_connect(conn, host, user, passwd, dbname, port, NULL, option) == NULL)
	{
		fprintf(stderr, "%s\n", mysql_error(conn));
		my_print_err_with_exit(conn, __FUNCTION__);
	}
}

void my_exec_query(MYSQL *conn, MYSQL_RES **res, const char *query, bool saveres)
{
	if (mysql_query(conn, query))
	{
		fprintf(stderr, "query failed: %s\n", mysql_error(conn));
		my_print_err_with_exit(conn, __FUNCTION__);
	}

	if (saveres)
	{
		*res = mysql_store_result(conn);
		if (*res == NULL)
		{
			fprintf(stderr, "result save failed: %s\n", mysql_error(conn));
			my_print_err_with_exit(conn, __FUNCTION__);
		}
	}
}

void my_print_coninfo(MYSQL_RES *res)
{
	int num_fields = mysql_num_fields(res); // 컬럼 개수 확인
	MYSQL_FIELD *fields = mysql_fetch_fields(res);  // 컬럼 메타데이터

	int i = 0;
	for (i = 0; i < num_fields; i++)
	{
		printf("%s(%d) \t", fields[i].name, fields[i].type);
		switch (fields[i].type)
		{
			case FIELD_TYPE_VAR_STRING:
				printf("type varchar \n");
				break;
			case FIELD_TYPE_STRING:
				printf("type string \n");
				break;
			case FIELD_TYPE_TINY:
				printf("type bool or tinyint \n");	// bool형은 tinyint(1) 로 처리되어서 정확히 판단을 못 함. 값으로 확인해야 할 듯
				break;
			case FIELD_TYPE_LONG:
				printf("type long \n");
				break;
			case FIELD_TYPE_DOUBLE:
				printf("type double \n");
				break;
			case FIELD_TYPE_DATE:
				printf("type date \n");
				break;
			case FIELD_TYPE_TIMESTAMP:
				printf("type timestamp \n");
				break;
			default:
				printf("type is unknown \n");
		}
	}
	printf("\n--------------------------------\n");
}

void my_print_res(MYSQL_RES *res)
{
	MYSQL_ROW row;

	int i = 0;
	while ((row = mysql_fetch_row(res)))
	{
		for (i = 0; i < mysql_num_fields(res); i++)
		{
			printf("%s \t", row[i] ? row[i] : "NULL");
		}
		printf("\n");
	}
}
