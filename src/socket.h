/*
 * Steve Walsh <steve.walsh@sap.com>
 * Aidan Shribman <aidan.shribman@sap.com>
 * Benoit Hudzia <benoit.hudzia@sap.com>
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

#define MASTER_HPROC_ID 1

/* helper functions */
struct hecaioc_hproc *hproc_array_init(int,
                struct hecaioc_hproc *, int);

/* master functions */
struct client_connect_info {
        int                 client_sock;
        pthread_t           thread_id;
        int                 *master_sock;
};

void master_open(int, struct client_connect_info *);

int master_clients_register(int, struct hecaioc_hproc *,
                struct client_connect_info *);

int master_clients_connect(int, struct hecaioc_hproc *,
                struct client_connect_info *);

int master_clients_mmap(int, int, struct hecaioc_hmr *,
                struct client_connect_info *);

void master_close(int, struct client_connect_info *);

/* client functions */
int client_connect(struct sockaddr_in *, int);

int client_hproc_count_recv(int, int *);

int client_hproc_array_recv(int, int, struct hecaioc_hproc *);

int client_register_ack(int);

int client_hproc_add(int, int, int, int, struct hecaioc_hproc *);

int client_hmr_count_recv(int, int *);

int client_unmap_array_recv(int, int, struct hecaioc_hmr *);

int client_mmap_ack(int);

int client_assign_mem(void *, unsigned long, int, struct hecaioc_hmr *);

#endif /* SOCKET_H_ */

