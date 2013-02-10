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

int heca_register(struct svm_data *local_svm);

int heca_connect(int fd, int local_svm_id, int svm_count,
        struct svm_data *svm_array);

int heca_memory_map(int fd, int mr_count, struct unmap_data *unmap_array,
        int local_svm_id);

void heca_cleanup(int fd);

#endif /* LIBHECA_IOCTL_H_ */

