/*
 * Steve Walsh <steve.walsh@sap.com>
 */

#include <linux/heca.h>
#include <assert.h>
#include "socket.h"
#include "ioctl.h"

#define DSM_CHRDEV  "/dev/heca"

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

/* 
 * HACK: static_dsm_id is used to maintain existing behavioir of libheca (in
 * which the dsm is automatically removed when FD is closed).
 */
static __u32 static_dsm_id;

int heca_dsm_add(int fd, struct hecaioc_svm *local_svm) 
{
    int rc;
    struct hecaioc_dsm dsm;

    assert(!static_dsm_id);

    dsm.dsm_id = local_svm->dsm_id;
    dsm.local = local_svm->remote;

    DEBUG_PRINT("HECAIOC_DSM_ADD system call\n");
    rc = ioctl(fd, HECAIOC_DSM_ADD, &dsm);
    if (rc) {
        DEBUG_ERROR("HECAIOC_DSM_INIT");
        return -1;
    }
    static_dsm_id = dsm.dsm_id;

    DEBUG_PRINT("HECAIOC_SVM_ADD (local) system call\n");
    rc = ioctl(fd, HECAIOC_SVM_ADD, local_svm);
    if (rc) {
        DEBUG_ERROR("HECAIOC_SVM_ADD (local)");
        return -1;
    }
    
    return 0;
}

int heca_svm_add(int fd, int local_svm_id, int svm_count,
        struct hecaioc_svm *svm_array)
{
    int i, rc;
    struct hecaioc_svm *svm;
    
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

int heca_mr_add(int fd, int mr_count, struct hecaioc_mr *unmap_array)
{
    int i, rc = 0;
    struct hecaioc_mr mr;

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

int heca_ps_pushback(int fd, int count, struct hecaioc_ps *array)
{
    int i, rc = 0;
    struct hecaioc_ps ps;

    for (i = 0; i < count; i++) {
        ps = array[i];

        DEBUG_PRINT("HECAIOC_PS_PUSHBACK system call\n");
        rc = ioctl(fd, HECAIOC_PS_PUSHBACK, &ps);
        if (rc < 0) {
            DEBUG_ERROR("HECAIOC_PS_PUSHBACK");
            return rc;
        }
    }
    return 0;
}

void heca_close(int fd)
{
    struct hecaioc_dsm dsm = {
        .dsm_id = static_dsm_id,
    };

    assert(static_dsm_id);
    DEBUG_PRINT("HECAIOC_DSM_RM system call\n");
    if (ioctl(fd, HECAIOC_DSM_RM, &dsm))
        DEBUG_ERROR("HECAIOC_DSM_RM");
    close(fd);
    static_dsm_id = 0;
}

