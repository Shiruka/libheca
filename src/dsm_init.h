/*
 * Steve Walsh <steve.walsh@sap.com>
 */

#ifndef DSM_INIT_DEF_H_
#define DSM_INIT_DEF_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <linux/dsm.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

#ifdef DEBUG
#define DEBUG_TEST 1
#else
#define DEBUG_TEST 0
#endif

#define DEBUG_PRINT(fmt, args...) \
        do { if (DEBUG_TEST) fprintf(stderr, "%s:%d:%s(): " fmt, \
                __FILE__, __LINE__, __FUNCTION__, ##args); } while (0)

#define DEBUG_ERROR(str) DEBUG_PRINT("ERROR: %s: %s\n", str, strerror(errno))

#define FALSE 0
#define TRUE 1
#define UNMAP_LOCAL 0
#define UNMAP_REMOTE 1

/* TODO: Make configurable */
#define DSM_CHRDEV  "/dev/rdma"
#define MASTER_SVM_ID 1
#define TCP_SIGNAL_SOCKET_PORT 4445

enum Signals {
    CLIENT_REGISTERED = 100, 
    CLIENT_CONNECT,
    CLIENT_CONNECTED,
    CLIENT_MEMORY_MAPPED
    };

struct client_connect_info 
{
    int                 client_sock;
    pthread_t           thread_id;
    int                 *master_sock;
};

int dsm_register(struct svm_data *local_svm);

void clients_sockets_init(int svm_count, struct client_connect_info *clients);

int dsm_clients_register(int svm_count, struct svm_data* svm_array,
        struct client_connect_info *clients);

int dsm_connect(int fd, int local_svm_id, int svm_count,
        struct svm_data *svm_array);

int dsm_clients_connect(int svm_count, struct svm_data *svm_array,
        struct client_connect_info *clients);

int dsm_memory_map(int fd, int mr_count, struct unmap_data *unmap_array,
        int local_svm_id, int auto_unmap);

int dsm_clients_memory_map(int svm_count, int mr_count,
        struct unmap_data *unmap_array, struct client_connect_info *clients);

void clients_socket_cleanup(int svm_count, struct client_connect_info *clients);

int dsm_master_connect(struct sockaddr_in *master_addr, int svm_id);

int dsm_svm_count_recv(int sock, int *svm_count);

int dsm_svm_array_recv(int sock, int svm_count, struct svm_data *svm_array);

int dsm_client_registered(int sock);

struct svm_data *dsm_local_svm_array_init(int svm_count,
        struct svm_data *svm_array, int local_svm_id);

int dsm_client_connect(int sock, int fd, int local_svm_id, int svm_count,
        struct svm_data *svm_array);

int dsm_mr_count_recv(int sock, int *mr_count);

int dsm_unmap_array_recv(int sock, int svm_count,
        struct unmap_data *unmap_array);

int dsm_client_memory_mapped(int sock);

int dsm_client_assign_mem(void *dsm_mem, unsigned long dsm_mem_sz, int mr_count,
        struct unmap_data *mr_array);

#endif /* DSM_INIT_DEF_H_ */

