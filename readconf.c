#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <libgen.h>

#include "readconf.h"

// remove spaces and new line characters
void trim(char *str)
{
    char *end = NULL;

    // front spaces
    while (*str == ' ' || *str == '\t')
    {
        str++;
    }
    end = str + strlen(str) - 1;

    // back spaces
    while (end > str && (*end == ' ' || *end == '\n' || *end == '\r' || *end == '\t'))
    {
        end--;
    }
    *(end + 1) = '\0';
}

// remove all spaces
void remove_spaces(char *str)
{
    char *src = str, *dst = str;
    while (*src)
    {
        if (*src != ' ' && *src != '\t')
        {
            *dst++ = *src;
        }
        src++;
    }
    *dst = '\0';
}

// validation of 'config.conf'
int validate_config_file(const char *filename)
{
    // only root
    if (getuid() != 0)
    {
        return 0;
    }

    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        return 0;
    }

    char line[MAX_LINE_LENGTH] = {0,};
    int valid = 1;
    while (fgets(line, sizeof(line), file) != NULL)
    {
        trim(line);
        // pass an empty line
        if (strlen(line) == 0)
        {
            continue;
        }

        // if not include '='
        if (strchr(line, '=') == NULL)
        {
            valid = 0;
            break;
        }
    }

    fclose(file);
    if (!valid)
    {
        // do somethings
    }
    return valid;
}

// get value of key
void* get_config_value(const char *filename, const char *key, CONF_TYPE type)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        return NULL;
    }

    char line[MAX_LINE_LENGTH] = {0,};
    while (fgets(line, sizeof(line), file) != NULL)
    {
        // find '='
        char *equals_sign = strchr(line, '=');
        if (equals_sign != NULL)
        {
            *equals_sign = '\0';
            char *config_key = line;
            char *config_value = equals_sign + 1;

            trim(config_key);
            trim(config_value);

            if (strcmp(config_key, key) == 0)
            {
                fclose(file);
                switch (type)
                {
                    case TYPE_STRING:
                    {
                        char *value_copy = strdup(config_value);
                        remove_spaces(value_copy);
                        return value_copy;
                    }
                    case TYPE_INT:
                    {
                        int *int_value = malloc(sizeof(int));
                        *int_value = atoi(config_value);
                        return int_value;
                    }
                    case TYPE_BOOL:
                    {
                        int *bool_value = malloc(sizeof(int));
                        *bool_value = (strcmp(config_value, "true") == 0 || strcmp(config_value, "1") == 0) ? 1 : 0;  
                        return bool_value;
                    }
                    default:
                    {
                        fclose(file);
                        return NULL;
                    }
                }
            }
        }

    }

    fclose(file);
    return NULL;
}

// the execution path of the process
void get_execute_path(char* path, size_t size)
{
    ssize_t len = readlink("/proc/self/exe", path, size - 1);
    if (len != -1)
    {
        path[len] = '\0';
        strcpy(path, dirname(path));
    }
}