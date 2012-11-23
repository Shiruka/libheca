/*
 * Steve Walsh <steve.walsh@sap.com>
 */

#include "libheca.h"

int dsm_master_init(int svm_count, struct svm_data *svm_array, int mr_count,
    struct unmap_data *mr_array, int auto_unmap)
{
    int fd, ret;
    struct svm_data *local_svm;
    struct client_connect_info *clients = NULL;

    local_svm = dsm_local_svm_array_init(svm_count, svm_array, MASTER_SVM_ID);
    clients = calloc(svm_count-1, sizeof(struct client_connect_info));
    clients_sockets_init(svm_count, clients);

    fd = dsm_register(local_svm);
    if ( fd  < 0 )
        goto return_error;
    
    ret = dsm_clients_register(svm_count, svm_array, clients);
    if ( ret < 0)
        goto return_error;
    
    ret = dsm_connect(fd, MASTER_SVM_ID, svm_count, svm_array); 
    if ( ret < 0)
        goto return_error;
    
    ret = dsm_clients_connect(svm_count, svm_array, clients);
    if ( ret < 0)
        goto return_error;

    ret = dsm_memory_map(fd, mr_count, mr_array, MASTER_SVM_ID, auto_unmap); 
    if ( ret < 0)
        goto return_error;

    ret = dsm_clients_memory_map(svm_count, mr_count, mr_array, clients); 
    if ( ret < 0)
        goto return_error;

    clients_socket_cleanup(svm_count, clients);

    return fd;    

return_error:
    DEBUG_ERROR("Failed to initialize master node");
    return ret;
}

int dsm_client_init(void *dsm_mem, unsigned long dsm_mem_sz, int local_svm_id,
        struct sockaddr_in *master_addr, int auto_unmap)
{
    int sock, svm_count, fd, ret, mr_count;
    struct svm_data *local_svm;
    struct svm_data *svm_array;
    struct unmap_data *mr_array;

    /* initial handshake, receive cluster data */
    sock = dsm_master_connect(master_addr, local_svm_id);
    if ( sock < 0)
        goto return_error;

    ret = dsm_svm_count_recv(sock, &svm_count); 
    if ( ret < 0)
        goto return_error;

    svm_array = calloc(svm_count, sizeof(struct svm_data));

    ret = dsm_svm_array_recv(sock, svm_count, svm_array);
    if ( ret < 0)
        goto return_error;

    local_svm = dsm_local_svm_array_init(svm_count, svm_array, local_svm_id);

    /* registration and connection */
    fd = dsm_register(local_svm); 
    if ( fd < 0)
        goto return_error;

    ret = dsm_client_registered(sock); 
    if ( ret < 0)
        goto return_error;

    ret = dsm_client_connect(sock, fd, local_svm_id, svm_count, svm_array); 
    if ( ret < 0)
        goto return_error;

    /* memory regions */
    ret = dsm_mr_count_recv(sock, &mr_count); 
    if ( ret < 0)
        goto return_error;

    mr_array = calloc(mr_count, sizeof(struct unmap_data));
    ret = dsm_unmap_array_recv(sock, mr_count, mr_array); 
    if ( ret < 0)
        goto return_error;

    ret = dsm_client_assign_mem(dsm_mem, dsm_mem_sz, mr_count, mr_array); 
    if ( ret < 0)
        goto return_error;

    ret = dsm_memory_map(fd, mr_count, mr_array, local_svm_id, auto_unmap); 
    if ( ret < 0)
        goto return_error;

    dsm_client_memory_mapped(sock);    
    
    free(svm_array);
    free(mr_array);
    close(sock);
    
    return fd;  

return_error:
    DEBUG_ERROR("Failed to initialize client node");
    return ret;
}

void dsm_cleanup(int fd)
{
    close(fd);
}
