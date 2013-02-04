#ifndef CONF_DSM_H_
#define CONF_DSM_H_

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "linux/dsm.h"
#include "libheca.h"
#include "config.h"

#define COMPUTE_ID 1
#define COMPUTE_ID_STR "1"

int init_cvm(struct CONF *, struct unmap_data *, int);
int init_mvm(unsigned long, void *, struct CONF *, int);

#endif

