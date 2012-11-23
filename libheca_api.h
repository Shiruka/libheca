/*
 * Steve Walsh <steve.walsh@sap.com>
 */
 
#ifndef LIB_HECA_DEF_H_
#define LIB_HECA_DEF_H_

#include <linux/dsm.h>

int dsm_master_init (int svm_count, struct svm_data *svm_array, int mr_count,
        struct unmap_data *mr_array);

int dsm_client_init (void *dsm_mem, unsigned long dsm_mem_sz, int local_svm_id,
        struct sockaddr_in *master_addr);

void dsm_cleanup(int fd);

#endif /* LIB_HECA_DEF_H_ */
