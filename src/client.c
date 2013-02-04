/*
 * Steve Walsh <steve.walsh@sap.com>
 */
#include "libheca.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PAGE_SIZE 4096

void notify(char *msg)
{
    char x;

    fprintf(stdout, "%s ... <press any key>", msg);
    fscanf(stdin, "%c", &x);
}

int main(int argc, char** argv) 
{    
    int fd, i;
    unsigned long dsm_mem_sz;
    void *dsm_mem;
    int local_svm_id;
    struct sockaddr_in master_addr;
    char * master_ip;
    
    if (argc < 3) {
        printf("Usage: ./client <id> <master_ip>\n");
        exit(1);
    }
    
    dsm_mem_sz = PAGE_SIZE * 1000;
    dsm_mem = valloc(dsm_mem_sz);
    local_svm_id = atoi(argv[1]);
    master_ip = argv[2];
    bzero((char*) &master_addr, sizeof(master_addr));
    master_addr.sin_family = AF_INET;
    master_addr.sin_port = htons(4445);
    master_addr.sin_addr.s_addr = inet_addr(master_ip);
    
    // dsm init
    fd = dsm_client_init (dsm_mem, dsm_mem_sz, local_svm_id, &master_addr, AUTO_UNMAP);
    if (fd < 0 ) {
        printf("Error initializing client node\n");
        exit(1);
    }
    
    /* DSM is now setup and ready to use */
    
    int pages = 5;
    
    notify("Init pages on client as 'm'");
    memset(dsm_mem, 'm', PAGE_SIZE * pages);
    for (i = 0; i < pages; i++) printf("%04d: %10.10s\n", i, (char *) dsm_mem+(PAGE_SIZE*i));
 
    notify("print pages on client after push back ( repatriates the one that weren't pushed");
    for (i = 0; i < pages; i++) printf("%04d: %10.10s\n", i, (char *) dsm_mem+(PAGE_SIZE*i));

    notify("disconnect");
    dsm_cleanup(fd); 
    
    return 0;
}

