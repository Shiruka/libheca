/*
 * Steve Walsh <steve.walsh@sap.com>
 * Aidan Shribman <aidan.shribman@sap.com>
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

/* helper functions */
struct hecaioc_svm *svm_array_init(int svm_count,
        struct hecaioc_svm *svm_array, int local_svm_id);

/* master functions */
struct client_connect_info {
    int                 client_sock;
    pthread_t           thread_id;
    int                 *master_sock;
};

void master_open(int svm_count, struct client_connect_info *clients);

int master_clients_register(int svm_count, struct hecaioc_svm *svm_array,
        struct client_connect_info *clients);

int master_clients_connect(int svm_count, struct hecaioc_svm *svm_array,
        struct client_connect_info *clients);

int master_clients_mmap(int svm_count, int mr_count,
        struct hecaioc_mr *unmap_array, struct client_connect_info *clients);

void master_close(int svm_count, struct client_connect_info *clients);

/* client functions */
int client_connect(struct sockaddr_in *master_addr, int svm_id);

int client_svm_count_recv(int sock, int *svm_count);

int client_svm_array_recv(int sock, int svm_count, struct hecaioc_svm
        *svm_array);

int client_register_ack(int sock);

int client_svm_add(int sock, int fd, int local_svm_id, int svm_count,
        struct hecaioc_svm *svm_array);

int client_mr_count_recv(int sock, int *mr_count);

int client_unmap_array_recv(int sock, int svm_count,
        struct hecaioc_mr *unmap_array);

int client_mmap_ack(int sock);

int client_assign_mem(void *dsm_mem, unsigned long dsm_mem_sz, int mr_count,
        struct hecaioc_mr *mr_array);

#endif /* SOCKET_H_ */

