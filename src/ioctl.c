/*
 * Steve Walsh <steve.walsh@sap.com>
 * Benoit Hudzia <benoit.hudzia@sap.com>
 */

#include <linux/heca.h>
#include <assert.h>
#include "socket.h"
#include "ioctl.h"

#define HECA_CHRDEV  "/dev/heca"

int heca_open(void)
{
        int fd;

        fd = open(HECA_CHRDEV, O_RDWR);
        int optval = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, 4);
        if (fd < 0) {
                DEBUG_ERROR("Could not open HECA_CHRDEV");
                return -1;
        }
        return fd;
}

/* 
 * HACK: static_dsm_id is used to maintain existing behavioir of libheca (in
 * which the dsm is automatically removed when FD is closed).
 */
static __u32 static_hspace_id;

int heca_hspace_add(int fd, struct hecaioc_hproc *local_hproc)
{
        int rc;
        struct hecaioc_hspace hspace;

        assert(!static_hspace_id);

        hspace.hspace_id = local_hproc->hspace_id;
        hspace.local = local_hproc->remote;

        DEBUG_PRINT("HECAIOC_HSPACE_ADD system call\n");
        rc = ioctl(fd, HECAIOC_HSPACE_ADD, &hspace);
        if (rc) {
                DEBUG_ERROR("HECAIOC_HSPACE_INIT");
                return -1;
        }
        static_hspace_id = hspace.hspace_id;

        DEBUG_PRINT("HECAIOC_PROC_ADD (local) system call\n");
        rc = ioctl(fd, HECAIOC_HPROC_ADD, local_hproc);
        if (rc) {
                DEBUG_ERROR("HECAIOC_HPROC_ADD (local)");
                return -1;
        }

        return 0;
}

int heca_hproc_add(int fd, int local_hproc_id, int hproc_count,
                struct hecaioc_hproc *hproc_array)
{
        int i, rc;
        struct hecaioc_hproc *hproc;

        for (i = 0; i < hproc_count; i++) {

                if (i == local_hproc_id-1)
                        continue; // only connect to remote svms

                hproc = &hproc_array[i];

                DEBUG_PRINT("HECAIOC_HPROC_ADD (remote)\n");
                rc = ioctl(fd, HECAIOC_HPROC_ADD, hproc);
                if (rc) {
                        DEBUG_ERROR("HECAIOC_HPROC_ADD (remote)");
                        return -1;
                }
        }
        return 0;
}

int heca_hmr_add(int fd, int hmr_count, struct hecaioc_hmr *unmap_array)
{
        int i, rc = 0;
        struct hecaioc_hmr hmr;

        for (i = 0; i < hmr_count; i++) {
                hmr = unmap_array[i];

                DEBUG_PRINT("HECAIOC_HMR_ADD system call\n");
                rc = ioctl(fd, HECAIOC_HMR_ADD, &hmr);
                if (rc < 0) {
                        DEBUG_ERROR("HECAIOC_HMR_ADD");
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
        struct hecaioc_hspace hspace = {
                .hspace_id = static_hspace_id,
        };

        assert(static_hspace_id);

	struct hecaioc_hmr hmr = {
		.hspace_id = static_hspace_id,
		.hmr_id = 4,
	};
	DEBUG_PRINT("HECAIOC_HMR_RM system call\n");
        if (ioctl(fd, HECAIOC_HMR_RM, &hmr))
                DEBUG_ERROR("HECAIOC_HMR_RM");

        DEBUG_PRINT("HECAIOC_HSPACE_RM system call\n");
        if (ioctl(fd, HECAIOC_HSPACE_RM, &hspace))
                DEBUG_ERROR("HECAIOC_HSPACE_RM");
        close(fd);
        static_hspace_id = 0;
}

