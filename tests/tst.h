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
#define SCALE_FACTOR 10
#define NUM_PAGES (25 * SCALE_FACTOR)
#define NUM_PUSHBACK SCALE_FACTOR

static struct hecaioc_mr mr_array[] = {
    {.mr_id = 1, .dsm_id = 1, .sz = PAGE_SIZE*NUM_PAGES, .flags = UD_AUTO_UNMAP},
    {.mr_id = 2, .dsm_id = 1, .sz = PAGE_SIZE*NUM_PAGES, .flags = UD_AUTO_UNMAP},
    {.mr_id = 3, .dsm_id = 1, .sz = PAGE_SIZE*NUM_PAGES, .flags = UD_AUTO_UNMAP},
    {.mr_id = 4, .dsm_id = 1, .sz = PAGE_SIZE*NUM_PAGES, .flags = UD_AUTO_UNMAP},
};
static int mr_count = sizeof(mr_array) / sizeof(*mr_array);
#define for_each_mr(i) \
    for (i = 0; i < mr_count; i++)

static inline void notify(char *msg)
{
    char x;

    fprintf(stdout, "%s", msg);
    fscanf(stdin, "%c", &x);
}

#endif
