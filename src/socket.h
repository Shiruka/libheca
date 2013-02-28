/*
 * Steve Walsh <steve.walsh@sap.com>
 */

#ifndef SOCKET_H_
#define SOCKET_H_

#include <linux/heca.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

#define MASTER_SVM_ID 1

struct client_connect_info {
    int                 client_sock;
    pthread_t           thread_id;
    int                 *master_sock;
};

void clients_sockets_init(int svm_count, struct client_connect_info *clients);

int heca_clients_register(int svm_count, struct svm_data* svm_array,
        struct client_connect_info *clients);

int heca_clients_connect(int svm_count, struct svm_data *svm_array,
        struct client_connect_info *clients);

int heca_clients_memory_map(int svm_count, int mr_count,
        struct unmap_data *unmap_array, struct client_connect_info *clients);

void clients_socket_cleanup(int svm_count, struct client_connect_info *clients);

int heca_master_connect(struct sockaddr_in *master_addr, int svm_id);

int heca_svm_count_recv(int sock, int *svm_count);

int heca_svm_array_recv(int sock, int svm_count, struct svm_data *svm_array);

int heca_client_registered(int sock);

struct svm_data *heca_local_svm_array_init(int svm_count,
        struct svm_data *svm_array, int local_svm_id);

int heca_client_connect(int sock, int fd, int local_svm_id, int svm_count,
        struct svm_data *svm_array);

int heca_mr_count_recv(int sock, int *mr_count);

int heca_unmap_array_recv(int sock, int svm_count,
        struct unmap_data *unmap_array);

int heca_client_memory_mapped(int sock);

int heca_client_assign_mem(void *dsm_mem, unsigned long dsm_mem_sz, int mr_count,
        struct unmap_data *mr_array);

#endif /* SOCKET_H_ */

