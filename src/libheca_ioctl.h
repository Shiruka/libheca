/*
 * Steve Walsh <steve.walsh@sap.com>
 * Aidan Shribman <aidan.shribman@sap.com>
 */

#ifndef LIBHECA_IOCTL_H_
#define LIBHECA_IOCTL_H_

#include <linux/dsm.h>

#ifdef DEBUG
#define DEBUG_PRINT(fmt, args...) \
    do { fprintf(stderr, "%s:%d:%s(): " fmt, \
        __FILE__, __LINE__, __FUNCTION__, ##args); } while (0)
#else
#define DEBUG_PRINT(fmt, args...) \
    do {} while (0)
#endif

#define DEBUG_ERROR(str) DEBUG_PRINT("ERROR: %s: %s\n", str, strerror(errno))

int heca_open(void);

void heca_close(int fd);

int heca_dsm_init(int fd, struct svm_data *local_svm);

int heca_svm_add(int fd, int local_svm_id, int svm_count, struct svm_data
        *svm_array);

int heca_mr_add(int fd, int mr_count, struct unmap_data *unmap_array, int
        local_svm_id);

#endif /* LIBHECA_IOCTL_H_ */

