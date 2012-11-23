#include "../headers/config.h"

struct CONF *config_parse(char *fname)
{
    FILE *f;
    char line[80], *buf1, *buf2;
    struct CONF *conf = NULL, **it = &conf;

    f = fopen(fname, "rt");
    while (fgets(line, 80, f) != NULL) {
        if (strstr(line, "#") == line)
            continue;

        buf1 = strstr(line, "=");
        if (!buf1)
            continue;
        buf2 = strstr(line, "\n");
        if (!buf2)
            continue;

        *it = malloc(sizeof(struct CONF));
        (*it)->key = malloc(sizeof(char)*(buf1-line+1));
        (*it)->value = malloc(sizeof(char)*(buf2-buf1));
        (*it)->next = NULL;

        strncpy((*it)->key, line, buf1-line);
        (*it)->key[buf1-line] = '\0';
        strncpy((*it)->value, buf1+1, buf2-buf1-1);
        (*it)->value[buf2-buf1-1] = '\0';

        it = &(*it)->next;
    }
    fclose(f);

    return conf;
}

void config_clean(struct CONF *conf)
{
    struct CONF *it = conf, *next;

    while (it) {
        next = it->next;
        free(it);
        it = next;
    }
}

char *config_get(struct CONF *conf, char *key)
{
    struct CONF *it;

    for (it = conf; it; it = it->next) {
        if (!strcmp(it->key, key))
            return it->value;
    }

    return NULL;
}

int config_get_int(struct CONF *conf, char *key)
{
    char *val = config_get(conf, key);
    if (!val)
        return 0;
    return atoi(val);
}

/* get up to max records with single-digit integer keys, in acsending order */
int config_get_ints(struct CONF *conf, int *keys, char **values, int max)
{
    int i, count = 0;
    char buf[2], *val;

    for (i = 0; i < 10; i++) {
        sprintf(buf, "%d", i+1);
        val = config_get(conf, buf);
        if (val) {
            values[count] = val;
            keys[count] = i+1;
            count++;
            if (count == max)
                break;
        }
    }

    return count;
}

