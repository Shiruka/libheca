/*
 * Steve Walsh <steve.walsh@sap.com>
 */

#include <linux/dsm.h>
#include "libheca_socket.h"
#include "libheca_ioctl.h"

#define DSM_CHRDEV  "/dev/rdma"

int heca_open(void)
{
    int fd;
   
    fd = open(DSM_CHRDEV, O_RDWR);
    int optval = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, 4);
    if (fd < 0) {
        DEBUG_ERROR("Could not open DSM_CHRDEV");
        return -1;
    }
    return fd;
}

int heca_dsm_init(int fd, struct svm_data *local_svm) 
{
    int rc;

    DEBUG_PRINT("HECAIOC_DSM_INIT system call\n");
    rc = ioctl(fd, HECAIOC_DSM_INIT, local_svm);
    if (rc) {
        DEBUG_ERROR("HECAIOC_DSM_INIT");
        return -1;
    }
    DEBUG_PRINT("HECAIOC_SVM_ADD (local) system call\n");
    rc = ioctl(fd, HECAIOC_SVM_ADD, local_svm);
    if (rc) {
        DEBUG_ERROR("HECAIOC_SVM_ADD (local)");
        return -1;
    }
    
    return 0;
}

int heca_svm_add(int fd, int local_svm_id, int svm_count,
        struct svm_data *svm_array)
{
    int i, rc;
    struct svm_data *svm;
    
    for (i = 0; i < svm_count; i++) {
        
        if (i == local_svm_id-1)
            continue; // only connect to remote svms
        
        svm = &svm_array[i];

        DEBUG_PRINT("HECAIOC_SVM_ADD (remote)\n");
        rc = ioctl(fd, HECAIOC_SVM_ADD, svm);
        if (rc) {
            DEBUG_ERROR("HECAIOC_SVM_ADD (remote)");
            return -1;
        }
    }
    return 0;
}

int heca_mr_add(int fd, int mr_count, struct unmap_data *unmap_array)
{
    int i, rc = 0;
    struct unmap_data mr;

    for (i = 0; i < mr_count; i++) {
        mr = unmap_array[i];

        DEBUG_PRINT("HECAIOC_MR_ADD system call\n");
        rc = ioctl(fd, HECAIOC_MR_ADD, &mr);
        if (rc < 0) {
            DEBUG_ERROR("HECAIOC_MR_ADD");
            return rc;
        }
    }
    return 0;
}

int heca_mr_pushback(int fd, int mr_count, struct unmap_data *unmap_array)
{
    int i, rc = 0;
    struct unmap_data mr;

    for (i = 0; i < mr_count; i++) {
        mr = unmap_array[i];

        DEBUG_PRINT("HECAIOC_MR_PUSHBACK system call\n");
        rc = ioctl(fd, HECAIOC_MR_PUSHBACK, &mr);
        if (rc < 0) {
            DEBUG_ERROR("HECAIOC_MR_PUSHBACK");
            return rc;
        }
    }
    return 0;
}

void heca_close(int fd)
{
    close(fd);
}

