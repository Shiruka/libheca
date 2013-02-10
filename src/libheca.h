/*
 * Steve Walsh <steve.walsh@sap.com>
 */
 
#ifndef LIBHECA_H_
#define LIBHECA_H_

#include "libheca_socket.h"
#include "libheca_ioctl.h"

/*
    dsm_master_init: initializes the master node for a heca connection. The
    function requires a pre-initialized svm_arry and mr_array. The auto_unmap 
    flag tells libheca to automatically unmap the memory region when creating 
    it. There are some cases where the user may not way to automatically unmap 
    this memory region and do so at a later stage when more information is 
    known about the system.
 */
int dsm_master_init (int svm_count, struct svm_data *svm_array, int mr_count,
        struct unmap_data *mr_array);

/* 
    dsm_client_init: initializes the client node for a heca connection. 
 */
int dsm_client_init (void *dsm_mem, unsigned long dsm_mem_sz, int local_svm_id,
        struct sockaddr_in *master_addr);

void dsm_cleanup(int fd);

#endif /* LIBHECA_H_ */
