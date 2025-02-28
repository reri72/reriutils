#ifndef _READCONFC_H_
#define _READCONFC_H_

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_LINE_LENGTH 256

typedef enum
{
    TYPE_INT,
    TYPE_STRING,
    TYPE_BOOL
} CONF_TYPE;

void trim(char *str);
void remove_spaces(char *str);
int validate_config_file(const char *filename);
void* get_config_value(const char *filename, const char *key, CONF_TYPE type);
void get_execute_path(char* path, size_t size);

#ifdef __cplusplus
}
#endif

#endif
