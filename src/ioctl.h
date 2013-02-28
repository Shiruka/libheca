/*
 * Steve Walsh <steve.walsh@sap.com>
 * Aidan Shribman <aidan.shribman@sap.com>
 */

#ifndef IOCTL_H_
#define IOCTL_H_

#include <linux/heca.h>

#ifdef DEBUG
#define DEBUG_PRINT(fmt, args...) \
    do { fprintf(stderr, "%s:%d:%s(): " fmt, \
        __FILE__, __LINE__, __FUNCTION__, ##args); } while (0)
#else
#define DEBUG_PRINT(fmt, args...) \
    do {} while (0)
#endif

#define DEBUG_ERROR(str) DEBUG_PRINT("ERROR: %s: %s\n", str, strerror(errno))

#endif /* IOCTL_H_ */

