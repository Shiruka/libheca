/*
 * Steve Walsh <steve.walsh@sap.com>
 */

#include "libheca.h"
#include "socket.h"

int heca_master_open(int svm_count, struct svm_data *svm_array, int mr_count,
    struct unmap_data *mr_array)
{
    int fd, ret;
    struct svm_data *local_svm;
    struct client_connect_info *clients = NULL;

    local_svm = svm_array_init(svm_count, svm_array, MASTER_SVM_ID);
    clients = calloc(svm_count-1, sizeof(struct client_connect_info));
    master_open(svm_count, clients);

    fd = heca_open();
    if (fd  < 0 )
        goto return_error;
 
    ret = heca_dsm_init(fd, local_svm);
    if (ret  < 0 )
        goto return_error;
    
    ret = master_clients_register(svm_count, svm_array, clients);
    if ( ret < 0)
        goto return_error;
    
    ret = heca_svm_add(fd, MASTER_SVM_ID, svm_count, svm_array); 
    if (ret < 0)
        goto return_error;
    
    ret = master_clients_connect(svm_count, svm_array, clients);
    if (ret < 0)
        goto return_error;

    ret = heca_mr_add(fd, mr_count, mr_array);
    if (ret < 0)
        goto return_error;

    ret = master_clients_mmap(svm_count, mr_count, mr_array, clients); 
    if (ret < 0)
        goto return_error;

    master_close(svm_count, clients);

    return fd;    

return_error:
    return ret;
}

int heca_client_open(void *dsm_mem, unsigned long dsm_mem_sz, int local_svm_id,
        struct sockaddr_in *master_addr)
{
    int sock, svm_count, fd, ret, mr_count;
    struct svm_data *local_svm;
    struct svm_data *svm_array;
    struct unmap_data *mr_array;

    /* initial handshake, receive cluster data */
    sock = client_connect(master_addr, local_svm_id);
    if ( sock < 0)
        goto return_error;

    ret = client_svm_count_recv(sock, &svm_count); 
    if ( ret < 0)
        goto return_error;

    svm_array = calloc(svm_count, sizeof(struct svm_data));

    ret = client_svm_array_recv(sock, svm_count, svm_array);
    if ( ret < 0)
        goto return_error;

    local_svm = svm_array_init(svm_count, svm_array, local_svm_id);

    /* registration and connection */
    fd = heca_open();
    if (fd < 0)
        goto return_error;

    ret = heca_dsm_init(fd, local_svm); 
    if (ret < 0)
        goto return_error;

    ret = client_register_ack(sock); 
    if (ret < 0)
        goto return_error;

    ret = client_svm_add(sock, fd, local_svm_id, svm_count, svm_array); 
    if (ret < 0)
        goto return_error;

    /* memory regions */
    ret = client_mr_count_recv(sock, &mr_count); 
    if (ret < 0)
        goto return_error;

    mr_array = calloc(mr_count, sizeof(struct unmap_data));
    ret = client_unmap_array_recv(sock, mr_count, mr_array); 
    if ( ret < 0)
        goto return_error;

    ret = client_assign_mem(dsm_mem, dsm_mem_sz, mr_count, mr_array); 
    if ( ret < 0)
        goto return_error;

    ret = heca_mr_add(fd, mr_count, mr_array);
    if ( ret < 0)
        goto return_error;

    client_mmap_ack(sock);    
    
    free(svm_array);
    free(mr_array);
    close(sock);
    
    return fd;  

return_error:
    return ret;
}

