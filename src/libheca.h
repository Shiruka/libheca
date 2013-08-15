/*
 * libheca.h -- interface of the 'libheca' library
 *
 * Steve Walsh <steve.walsh@sap.com>
 * Aidan Shribman <aidan.shribman@sap.com>
 * Benoit Hudzia <benoit.hudzia@sap.com>
 */

#ifndef LIBHECA_H_
#define LIBHECA_H_

#include <netinet/in.h>
#include <linux/heca.h>

__BEGIN_DECLS

int heca_open(void);

void heca_close(int);

int heca_hspace_add(int, struct hecaioc_hproc *);

int heca_hproc_add(int, int, int, struct hecaioc_hproc *);

int heca_hmr_add(int, int, struct hecaioc_hmr *);

int heca_ps_pushback(int, int, struct hecaioc_ps *);

/*
 *heca_master_open: initializes the master node for a heca connection. The
 *function requires a pre-initialized svm_arry and mr_array. The auto_unmap 
 *flag tells libheca to automatically unmap the memory region when creating 
 *it. There are some cases where the user may not way to automatically unmap 
 *this memory region and do so at a later stage when more information is 
 *known about the system.
 */
int heca_master_open(int, struct hecaioc_hproc *, int, struct hecaioc_hmr *);

/* 
 *heca_client_open: initializes the client node for a heca connection. 
 */
int heca_client_open(void *, unsigned long, int, struct sockaddr_in *);

__END_DECLS

#endif /* LIBHECA_H_ */

