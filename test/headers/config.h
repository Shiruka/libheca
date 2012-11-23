#ifndef CONF_DEF_H_
#define CONF_DEF_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "linux/dsm.h"

struct CONF {
    char *key;
    char *value;
    struct CONF *next;
};

struct CONF *config_parse(char *);
void config_clean(struct CONF *);
char *config_get(struct CONF *, char *);
int config_get_int(struct CONF *, char *);
int config_get_ints(struct CONF *, int *, char **, int);

#endif

