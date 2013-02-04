/*
 * Steve Walsh <steve.walsh@sap.com>
 */
 
#ifndef LIB_HECA_DEF_H_
#define LIB_HECA_DEF_H_

#include "dsm_init.h"

#define AUTO_UNMAP 1
#define NO_AUTO_UNMAP 0

/*
    dsm_master_init: initializes the master node for a heca connection. The
    function requires a pre-initialized svm_arry and mr_array. The auto_unmap 
    flag tells libheca to automatically unmap the memory region when creating 
    it. There are some cases where the user may not way to automatically unmap 
    this memory region and do so at a later stage when more information is 
    known about the system.
 */
int dsm_master_init (int svm_count, struct svm_data *svm_array, int mr_count,
        struct unmap_data *mr_array, int auto_unmap);

/* 
    dsm_client_init: initializes the client node for a heca connection. 
 */
int dsm_client_init (void *dsm_mem, unsigned long dsm_mem_sz, int local_svm_id,
        struct sockaddr_in *master_addr, int auto_unmap);

void dsm_cleanup(int fd);

#endif /* LIB_HECA_DEF_H_ */
