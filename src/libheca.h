/*
 * Steve Walsh <steve.walsh@sap.com>
 * Aidan Shribman <aidan.shribman@sap.com>
 */
 
#ifndef LIBHECA_H_
#define LIBHECA_H_

#include "libheca_socket.h"
#include "libheca_ioctl.h"

/*
    heca_master_open: initializes the master node for a heca connection. The
    function requires a pre-initialized svm_arry and mr_array. The auto_unmap 
    flag tells libheca to automatically unmap the memory region when creating 
    it. There are some cases where the user may not way to automatically unmap 
    this memory region and do so at a later stage when more information is 
    known about the system.
 */
int heca_master_open(int svm_count, struct svm_data *svm_array, int mr_count,
        struct unmap_data *mr_array);

/* 
    heca_client_open: initializes the client node for a heca connection. 
 */
int heca_client_open(void *dsm_mem, unsigned long dsm_mem_sz, int local_svm_id,
        struct sockaddr_in *master_addr);

#endif /* LIBHECA_H_ */

