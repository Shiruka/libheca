#ifndef CONF_TST_H_
#define CONF_TST_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "dsm.h"
#include "config.h"

#define PAGE_SIZE 4096
#define NUM_PAGES 25000
#define NUM_PUSHBACK 1000

static struct unmap_data mr_array[] = {
    {.id = 1, .dsm_id = 1, .sz = PAGE_SIZE*NUM_PAGES},
    {.id = 2, .dsm_id = 1, .sz = PAGE_SIZE*NUM_PAGES},
    {.id = 3, .dsm_id = 1, .sz = PAGE_SIZE*NUM_PAGES},
    {.id = 4, .dsm_id = 1, .sz = PAGE_SIZE*NUM_PAGES},
};
static int mr_count = sizeof(mr_array) / sizeof(struct unmap_data);
#define for_each_mr(i) \
    for (i = 0; i < mr_count; i++)

static inline void notify(char *msg)
{
    char x;

    fprintf(stdout, "%s", msg);
    fscanf(stdin, "%c", &x);
}

#endif
