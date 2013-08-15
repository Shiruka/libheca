/*
 * Steve Walsh <steve.walsh@sap.com>
 * Aidan Shribman <aidan.shribman@sap.com>
 * Benoit Hudzia <benoit.hudzia@sap.com>
 */

#include <assert.h>
#include "libheca.h"
#include "socket.h"

int heca_master_open(int hproc_count, struct hecaioc_hproc *hproc_array,
                int hmr_count, struct hecaioc_hmr *hmr_array)
{
        int fd, ret;
        struct hecaioc_hproc *local_hproc;
        struct client_connect_info *clients = NULL;

        local_hproc = hproc_array_init(hproc_count, hproc_array,
                        MASTER_HPROC_ID);
        clients = calloc(hproc_count-1, sizeof(struct client_connect_info));
        master_open(hproc_count, clients);

        fd = heca_open();
        if (fd  < 0 )
                goto return_error;

        ret = heca_hspace_add(fd, local_hproc);
        if (ret  < 0 )
                goto return_error;

        ret = master_clients_register(hproc_count, hproc_array, clients);
        if ( ret < 0)
                goto return_error;

        ret = heca_hproc_add(fd, MASTER_HPROC_ID, hproc_count, hproc_array);
        if (ret < 0)
                goto return_error;

        ret = master_clients_connect(hproc_count, hproc_array, clients);
        if (ret < 0)
                goto return_error;

        ret = heca_hmr_add(fd, hmr_count, hmr_array);
        if (ret < 0)
                goto return_error;

        ret = master_clients_mmap(hproc_count, hmr_count, hmr_array, clients);
        if (ret < 0)
                goto return_error;

        master_close(hproc_count, clients);

        return fd;

return_error:
        return ret;
}

int heca_client_open(void *hspace_mem, unsigned long hspace_mem_sz,
                int local_hproc_id, struct sockaddr_in *master_addr)
{
        int sock, hproc_count, fd, ret, hmr_count;
        struct hecaioc_hproc *local_hproc;
        struct hecaioc_hproc *hproc_array;
        struct hecaioc_hmr *hmr_array;

        /* initial handshake, receive cluster data */
        sock = client_connect(master_addr, local_hproc_id);
        if ( sock < 0)
                goto return_error;

        ret = client_hproc_count_recv(sock, &hproc_count);
        if ( ret < 0)
                goto return_error;

        hproc_array = calloc(hproc_count, sizeof(*local_hproc));

        ret = client_hproc_array_recv(sock, hproc_count, hproc_array);
        if ( ret < 0)
                goto return_error;

        local_hproc = hproc_array_init(hproc_count, hproc_array,
                        local_hproc_id);

        /* registration and connection */
        fd = heca_open();
        if (fd < 0)
                goto return_error;

        ret = heca_hspace_add(fd, local_hproc);
        if (ret < 0)
                goto return_error;

        ret = client_register_ack(sock);
        if (ret < 0)
                goto return_error;

        ret = client_hproc_add(sock, fd, local_hproc_id, hproc_count,
                        hproc_array);
        if (ret < 0)
                goto return_error;

        /* memory regions */
        ret = client_hmr_count_recv(sock, &hmr_count);
        if (ret < 0)
                goto return_error;

        hmr_array = calloc(hmr_count, sizeof(*hmr_array));
        assert(hmr_array);
        ret = client_unmap_array_recv(sock, hmr_count, hmr_array);
        if ( ret < 0)
                goto return_error;

        ret = client_assign_mem(hspace_mem, hspace_mem_sz, hmr_count,
                        hmr_array);
        if ( ret < 0)
                goto return_error;

        ret = heca_hmr_add(fd, hmr_count, hmr_array);
        if ( ret < 0)
                goto return_error;

        client_mmap_ack(sock);

        free(hproc_array);
        free(hmr_array);
        close(sock);

        return fd;

return_error:
        return ret;
}

