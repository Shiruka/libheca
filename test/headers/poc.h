#ifndef POC_DEF_H_
#define POC_DEF_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <assert.h>

#include "dsm.h"
#include "config.h"
#include "quicksort.h"

static int (*payload)(void *, unsigned long);

#endif

