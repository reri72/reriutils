#ifndef _UTILES_H_
#define _UTILES_H_

#ifdef __cplusplus
extern "C" {
#endif

void nano_sleep(int sec, int nsec);
int check_rootuser();
int check_exist_file(const char *fullpath);

#ifdef __cplusplus
}
#endif

#endif