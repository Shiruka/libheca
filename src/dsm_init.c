/*
 * Steve Walsh <steve.walsh@sap.com>
 */

#include "dsm_init.h"

int dsm_register(struct svm_data *local_svm) 
{
    int rc, fd;
   
    fd = open(DSM_CHRDEV, O_RDWR);
    int optval = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, 4);
    if (fd < 0) {
        DEBUG_ERROR("Could not open DSM_CHRDEV");
        return -1;
    }

    DEBUG_PRINT("DSM_DSM system call\n");
    rc = ioctl(fd, DSM_DSM, local_svm);
    if (rc) {
        DEBUG_ERROR("DSM_DSM");
        return -1;
    }
    DEBUG_PRINT("DSM_SVM (local) system call\n");
    rc = ioctl(fd, DSM_SVM, local_svm);
    if (rc) {
        DEBUG_ERROR("DSM_SVM (local)");
        return -1;
    }
    
    return fd;
}

static int get_listening_socket(int client_count)
{
    int sockfd;
    struct sockaddr_in serv_addr;
       
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    int optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, 4);
    if (sockfd < 0) {
        DEBUG_ERROR("Could not open socket");
        return sockfd;
    }
    
    bzero((char*) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(TCP_SIGNAL_SOCKET_PORT);
    if (bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
        DEBUG_ERROR("Could not bind scoket");
        return -1;
    }
    
    listen(sockfd, client_count);
    
    return sockfd;
}

static void * client_socket_init(void *arg)
{
    int client_sock, n, svm_id;
    struct sockaddr_in cli_addr;     
    socklen_t clilen;
    struct client_connect_info *client;
    void *ret = NULL;
    
    client = (struct client_connect_info *) arg;    
    clilen = sizeof(cli_addr);
    
    DEBUG_PRINT("Waiting for client connection...\n");
    client_sock = accept(*(client->master_sock), (struct sockaddr*) &cli_addr, &clilen);
    if (client_sock < 0) 
        DEBUG_ERROR("DSM_DSM");
            
    /* backup socket connection */
    client->client_sock = client_sock;
    
    n = read(client_sock, &svm_id, sizeof(int));
    if (n < 0)
        DEBUG_ERROR("Could not read from socket");        
    DEBUG_PRINT("svm: %d connected\n", svm_id);
    
    return ret;
}

void clients_sockets_init(int svm_count, struct client_connect_info *clients)
{
    int i, client_count, listening_sock;    
    
    client_count = svm_count - 1;
    listening_sock = get_listening_socket(client_count);    

    /* start thread to receive connection from each client */
    for (i = 0; i < client_count; i++) {
        clients[i].master_sock = &listening_sock;
        pthread_create(&clients[i].thread_id, NULL, client_socket_init, &clients[i]);
    }
    
    /* Wait for clients to respond before progressing */
    for (i = 0; i < client_count; i++) {
        pthread_join(clients[i].thread_id, NULL);
    }
    
    close(listening_sock);
}

/* wait for clients to register */
int dsm_clients_register(int svm_count, struct svm_data *svm_array,
        struct client_connect_info *clients)
{  
    int i, client_count, n, ack;    
    client_count = svm_count -1;
   
    for (i = 0; i < client_count; i++) {

        n = write(clients[i].client_sock, &svm_count, sizeof(int));
        if (n <  0) {
            DEBUG_ERROR("Could not write to socket");
            return -1;
        }

        n = write(clients[i].client_sock, svm_array,
                svm_count * sizeof(struct svm_data));
        if (n <  0) {
            DEBUG_ERROR("Could not write to socket");
            return -1;
        }

        n = read(clients[i].client_sock, &ack, sizeof(int));
        if (n < 0) {
            DEBUG_ERROR("Could not read from socket"); 
            return -1;
        }

        if (ack != CLIENT_REGISTERED) {
            DEBUG_ERROR("Failed to register client");
            return -1;
        }
    }
    return 0;    
}

/* register and connect remote svms */
int dsm_connect(int fd, int local_svm_id, int svm_count,
        struct svm_data *svm_array)
{
    int i, rc;
    struct svm_data *svm;
    
    for (i = 0; i < svm_count; i++) {
        
        if (i == local_svm_id-1)
            continue; // only connect to remote svms
        
        svm = &svm_array[i];

        DEBUG_PRINT("DSM_SVM (remote)\n");
        rc = ioctl(fd, DSM_SVM, svm);
        if (rc) {
            DEBUG_ERROR("DSM_SVM (remote)");
            return -1;
        }

        DEBUG_PRINT("DSM_CONNECT\n");
        rc = ioctl(fd, DSM_CONNECT, svm);
        if (rc) {
            DEBUG_ERROR("DSM_CONNECT");
            return -1;
        }
    }
    return 0;
}

/* wait for clients to connect */
int dsm_clients_connect(int svm_count, struct svm_data *svm_array,
        struct client_connect_info *clients)
{
    int i, client_count, sig, n, ack;
    client_count = svm_count -1;
    sig = CLIENT_CONNECT;
    
    for (i = 0; i < client_count; i++) {
        
        /* send connect signal to client and get ack when client is connected */
        n = write(clients[i].client_sock, &sig, sizeof(int));
        if (n <  0) {
            DEBUG_ERROR("Could not write to socket");
            return -1;
        }
        
        n = read(clients[i].client_sock, &ack, sizeof(int));
        if (n < 0) {
            DEBUG_ERROR("Could not read from socket");
            return -1;
        }
        
        if (ack != CLIENT_CONNECTED) {
            DEBUG_ERROR("Client has not connected to other nodes");
            return -1;
        }
        
    }
    return 0;
}

/* client maps its valloc()ed memory into memory regions */
int dsm_client_assign_mem(void *dsm_mem, unsigned long dsm_mem_sz, int mr_count,
        struct unmap_data *mr_array)
{
    int i;
    void *pos = dsm_mem;

    for (i = 0; i < mr_count; i++) {
        if (pos > dsm_mem + dsm_mem_sz) {
            DEBUG_ERROR("Not enough memory allocate on client");
            return -1;
        }
        mr_array[i].addr = pos;
        pos += mr_array[i].sz;
    }

    return 0;
}

/* register memory regions, unmap to remotes if needed */
int dsm_memory_map(int fd, int mr_count, struct unmap_data *unmap_array,
        int local_svm_id, int auto_unmap)
{
    int i, j, rc = 0;
    struct unmap_data mr;

    for (i = 0; i < mr_count; i++)
    {
        mr = unmap_array[i];
        if (auto_unmap)
            mr.unmap = TRUE;
        else
            mr.unmap = FALSE;

        j = 0;
        while (mr.svm_ids[j] != 0) {
            if (local_svm_id == mr.svm_ids[j]) {
                mr.unmap = FALSE;
                break;
            }
            j++;
        }
        DEBUG_PRINT("DSM_MR system call\n");
        rc = ioctl(fd, DSM_MR, &mr);
        if (rc < 0) {
            DEBUG_ERROR("DSM_MR");
            return -1;
        }
    }
    return 0;
}

/* wait for clients to register memory regions */
/* FIXME: erase unneeded local pointers on unmap_array */
int dsm_clients_memory_map(int svm_count, int mr_count,
        struct unmap_data *unmap_array, struct client_connect_info *clients)
{
    int i, client_count, n, ack_sig;
    client_count = svm_count - 1;
    
    DEBUG_PRINT("Setting up clients memory mapping\n");

    for (i = 0; i < client_count; i++) {

        n = write(clients[i].client_sock, &mr_count, sizeof(int));
        if (n <  0) {
            DEBUG_ERROR("Could not write to socket");
            return n;
        }
        
        n = write(clients[i].client_sock, unmap_array,
                mr_count * sizeof(struct unmap_data));
        if (n <  0) {
            DEBUG_ERROR("Could not write to socket");
            return n;
        }
        
        // get ack
        n = read(clients[i].client_sock, &ack_sig, sizeof(int));
        if (n < 0) {
            DEBUG_ERROR("Could not read from socket");
            return n;
        }
        
        if (ack_sig != CLIENT_MEMORY_MAPPED) {
            DEBUG_ERROR("Failed to get CLIENT_MEMORY_MAPPED signal from client");
            return -1;
        }
    }
    return 0;
}

void clients_socket_cleanup(int svm_count, struct client_connect_info *clients)
{
    int i;

    for (i = 0; i < svm_count-1; i++ )
        close(clients[i].client_sock);

    free(clients);
}

/* client connects to master */
int dsm_master_connect(struct sockaddr_in *master_addr, int svm_id)
{
    int sockfd, n;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        DEBUG_ERROR("Could not open socket");
        return -1;
    }

    if (connect(sockfd, (struct sockaddr*) master_addr,
                sizeof(struct sockaddr)) < 0) {
        DEBUG_ERROR("Could not connect to master");
        return -1;
    }

    n = write(sockfd, &svm_id, sizeof(int));
    if (n <  0) {
        perror("ERROR could not write to socket");   
        return -1;
    }
    
    return sockfd;
}

int dsm_svm_count_recv(int sock, int *svm_count)
{
    int n = read(sock, svm_count, sizeof(int));
    if (n < 0) {
        DEBUG_ERROR("Could not read from socket");
        return -1;
    }
    
    DEBUG_PRINT("Received svm count: %d \n", *svm_count);
    return 0;
}

int dsm_svm_array_recv(int sock, int svm_count, struct svm_data *svm_array)
{
    int n = read(sock, svm_array, svm_count * sizeof(struct svm_data));

    if (n < 0) {
        DEBUG_ERROR("Could not read from socket");
        return n;
    }
    return 0;
}

int dsm_client_registered(int sock)
{
    int n, sig;

    sig = CLIENT_REGISTERED;    
    n = write(sock, &sig, sizeof(int));
    if (n < 0) {
        DEBUG_ERROR("Could not read from socket");
        return -1;
    }
    return 0;
}

/* set svm_data.local values, return svm_data for local svm */
struct svm_data *dsm_local_svm_array_init(int svm_count,
        struct svm_data *svm_array, int local_svm_id)
{
    int i;

    for (i = 0; i < svm_count; i++) {
        if (i == (local_svm_id - 1)) 
            svm_array[i].local = TRUE;
        else
            svm_array[i].local = FALSE;
    }
    
    return &svm_array[local_svm_id - 1];
}


int dsm_client_connect(int sock, int fd, int local_svm_id, int svm_count,
        struct svm_data *svm_array)
{
    int n, sig, ret;

    n = read(sock, &sig, sizeof(int));
    if (n < 0) {
        DEBUG_ERROR("Could not read from socket");
        return n;
    }
    
    if (sig != CLIENT_CONNECT) {
        DEBUG_ERROR("Failed to receive connect signal from master");
        return -1;
    }

    ret = dsm_connect(fd, local_svm_id, svm_count, svm_array);
    if (ret < 0) {
        return ret;
    }

    sig = CLIENT_CONNECTED;
    n = write(sock, &sig, sizeof(int));
    if (n < 0) {
        DEBUG_ERROR("Could not read from socket");
        return n;
    }
    return 0;
}

int dsm_mr_count_recv(int sock, int *mr_count)
{
    int n;
    n = read(sock, mr_count, sizeof(int));
    if (n < 0) {
        DEBUG_ERROR("Could not read from socket");
        return n;
    }
    
    DEBUG_PRINT("Received memory region count: %d \n", *mr_count);
    return 0;
}

int dsm_unmap_array_recv(int sock, int mr_count, struct unmap_data *unmap_array)
{
    int n;

    n = read(sock, unmap_array, mr_count * sizeof(struct unmap_data));
    if (n < 0) {
        DEBUG_ERROR("Could not read from socket"); 
        return n;
    }
    return 0;
}

int dsm_client_memory_mapped(int sock)
{
    int n, sig;
    
    sig = CLIENT_MEMORY_MAPPED;
    n = write(sock, &sig, sizeof(int));
    if (n < 0) {
        DEBUG_ERROR("Could not read from socket");
        return n;
    }
    return 0;
}
