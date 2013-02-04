/*
 * Steve Walsh <steve.walsh@sap.com>
 */
#include "libheca.h"

#define PAGE_SIZE 4096
#define NUM_PAGES 5

static struct svm_data svm_array[2] = {
    { .dsm_id = 1, .svm_id = 1, .ip = "192.168.4.1", .port = 4444 },
    { .dsm_id = 1, .svm_id = 2, .ip = "192.168.4.2", .port = 4444 }
};

static struct unmap_data mr_array[1] = {
    { .dsm_id = 1, .id = 1, .svm_ids = {2, 0} }
};

void notify(char *msg)
{
    char x;

    fprintf(stdout, "%s ... <press any key>", msg);
    fscanf(stdin, "%c", &x);
}

int main() 
{    
    int fd, i;
    unsigned long dsm_mem_sz = PAGE_SIZE * 1000;
    int svm_count = sizeof(svm_array) / sizeof(struct svm_data);
    int mr_count = sizeof(mr_array) / sizeof(struct unmap_data);
    void *mem = valloc(dsm_mem_sz);

    if (!mem) {
        printf("Could not allocate enough memory\n");
        exit(1);
    }

    /* allocate just one memory region */
    mr_array[0].addr = mem;
    mr_array[0].sz = dsm_mem_sz;

    fd = dsm_master_init(svm_count, svm_array, mr_count, mr_array, AUTO_UNMAP);
    if (fd < 0) {
        printf("Error initializing master node\n");
        exit(1);
    }
    
    /* DSM is now setup and ready to use */
    notify("get pages from client");
    for (i = 0; i < NUM_PAGES; i++)
        printf("%04d: %10.10s\n", i, (char *) mem+(PAGE_SIZE*i));

    notify("dirty pages locally on master and push back to client");
    memset(mem, 'c', PAGE_SIZE * NUM_PAGES);
    for (i = 0; i < NUM_PAGES; i++)
        printf("%04d: %10.10s\n", i, (char *) mem+(PAGE_SIZE*i));

    notify("disconnect");

    dsm_cleanup(fd);
    return 0;
}
