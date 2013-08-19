#include <sys/types.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "tst.h"

static void print_pages(unsigned long n)
{
    int i, j;

    for_each_mr (j) {
        for (i = 0; i < n; i++) {
            printf("%d:%d %10.10s\n", j, i,
                    (char *) mr_array[j].addr+(PAGE_SIZE*i));
        }
    }
}

static void dirty_pages(unsigned long n, char c)
{
    int i;

    for_each_mr (i)
        memset(mr_array[i].addr, c, n*PAGE_SIZE);
}

static void push_pages(pid_t child, int fd, unsigned long n)
{
    struct hecaioc_ps data;
    int k;

    bzero(&data, sizeof data);
    data.sz = PAGE_SIZE * n;
    data.pid = child;
    for_each_mr (k) {
        int j;
        data.addr = mr_array[k].addr;
        j = heca_ps_pushback(fd, 1, &data);
        if (j)
            printf("mr[%d] error in HECAIOC_PS_PUSHBACK: %d\n", k, j);
    }
}

static void compute(char *conf_name, int kflag, int tflag, int pflag, int lflag, int no_loops, int rest)
{
    int fd = -1, i, lockfd = -1, rc, ret1, ret2, j;
    struct CONF *conf = NULL;
    pid_t child;
    const char *lockfn = "/tmp/tst.lock";
    char *write = 'A';

    for_each_mr (i) {
        mr_array[i].addr = valloc(PAGE_SIZE*NUM_PAGES);

        if (kflag == 1) {
            ret1 = madvise(mr_array[i].addr, mr_count, MADV_MERGEABLE);
            if (ret1 < 0)
                perror("madvise error");
            notify("Finished");
	}
    
        if (tflag == 1) {
            ret2 = madvise(mr_array[i].addr, mr_count, MADV_HUGEPAGE);
            if (ret2 < 0)
                perror("madvise error");
	    notify("Finished");
        }
    }

    if (lflag == 1){
        for(j = 0; j < no_loops; j++) {        
            for_each_mr (i) {
                dirty_pages(NUM_PUSHBACK ,write);
                print_pages(NUM_PUSHBACK);
            }
            write++;
            sleep(rest);
        }
    
        notify("Finished");
    }   
#if 0
        mr_array[i].flags |= UD_COPY_ON_ACCESS;
#endif
   
    if (pflag == 1) {
        fprintf(stderr, "Parent (%d): started\n", getpid());

        if ((child = fork()) == -1) {
            perror("fork error");
            goto failure;
        } else if (!child) {
            if ((lockfd = open(lockfn, O_RDWR|O_CREAT|O_APPEND, 0666)) < 0) {
                perror("open");
                goto failure;
            }

            fprintf(stderr, "Child (%d): started\n", getpid());

            sleep(2);

            if (flock(lockfd, LOCK_EX) < 0) {
                perror("flock");
                goto failure;
            }

            notify("[1] pull all pages: ");
            print_pages(NUM_PAGES);

            if (flock(lockfd, LOCK_UN) < 0) {
                perror("flock");
                goto failure;
            }  

            sleep(1);

            if (flock(lockfd, LOCK_EX) < 0) {
                perror("flock");
                goto failure;
            }

            notify("[4] re-pull pages:");
            print_pages(NUM_PUSHBACK);

            notify("[5] dirty and print pages (1):");
            dirty_pages(NUM_PUSHBACK, '1');
            print_pages(NUM_PUSHBACK);

            notify("[7] dirty and print pages (3):");
            dirty_pages(NUM_PUSHBACK, '3');
            print_pages(NUM_PUSHBACK);

            if (flock(lockfd, LOCK_UN) < 0) {
                perror("flock");
                goto failure;
            }

            sleep(1);

            if (flock(lockfd, LOCK_EX) < 0) {
                perror("flock");
                goto failure;
            }

            notify("[.] disconnect:\n");
            fprintf(stderr, "Child (%d): ended\n", getpid());

            if (flock(lockfd, LOCK_UN) < 0) {
                perror("flock");
                goto failure;
            }

            goto done;
        }

        sleep(1);

        if ((lockfd = open(lockfn, O_RDWR|O_CREAT|O_APPEND, 0666)) < 0) {
            perror("open");
            goto failure;
        }   

        if (flock(lockfd, LOCK_EX) < 0) {
            perror("flock");
            goto failure;
        }  

        fprintf(stderr, "[0] initialize:\n");

        conf = config_parse(conf_name);
        assert(conf);
        fd = init_cvm(child, conf, mr_array, mr_count, 1);
        if (fd < 0) {
            fprintf(stderr, "can't open /dev/heca\n");
            goto failure;
        }

        fprintf(stderr, "Parent: completed HECA setup...\n");

        if (flock(lockfd, LOCK_UN) < 0) {
            perror("flock");
            goto failure;
        }

        sleep(1);

        if (flock(lockfd, LOCK_EX) < 0) {
            perror("flock");
            goto failure;
        }

        notify("[2] push back pages:");
        push_pages(child, fd, NUM_PUSHBACK);

        if (flock(lockfd, LOCK_UN) < 0) {
            perror("flock");
            goto failure;
        }

        /* parent */
        while (1) {
            int status;
            pid_t end;

            end = waitpid(child, &status, WNOHANG|WUNTRACED);
            if (end == -1) {
                perror("waitpid error");
                goto failure;
            } 
            else if (!end) {
                /* child still running */
                sleep(1);
            }
            else if (end == child) {
                if (WIFEXITED(status))
                    printf("Child ended normally\n");
                else if (WIFSIGNALED(status))
                    printf("Child ended because of an uncaught signal\n");
                else if (WIFSTOPPED(status))
                    printf("Child process has stopped\n");
                break;
            }
        }

        fprintf(stderr, "Parent (%d): is back\n", getpid());

        notify("[X] parent will now close /dev/heca");
        heca_close(fd);
        fd = -1;
        fprintf(stderr, "Parent (%d): ended\n", getpid());

        rc = 0;
        goto done;
    }
    /* child/parent cleanup */
failure:
    rc = 1;
done:
    if (conf)
        config_clean(conf);
    for_each_mr (i) {
        free(mr_array[i].addr);
        mr_array[i].addr = 0;
    }
    if (lockfd != -1)
        close(lockfd);
    exit(rc);
}

static void provide(char *conf_name, int mvm_id, char c, int kflag, int tflag, int pflag, int lflag, int no_loops, int rest)
{
    struct CONF *conf;
    unsigned long sz = PAGE_SIZE*NUM_PAGES*mr_count;
    int fd, i, ret1, ret2, j;
    void *mem;
    char *write = 'A';

    conf = config_parse(conf_name);
    assert(conf);

    mem = valloc(sz);
    assert(mem);
    memset(mem, c, sz);

    for_each_mr (i) {
        mr_array[i].addr = mem + (i * mr_array[i].sz);

        if (kflag == 1) {
            ret1 = madvise(mr_array[i].addr, mr_count, MADV_MERGEABLE);
            if (ret1 < 0)
                perror("madvise error");
	    notify("Finished");
        }

        if (tflag == 1) {
            ret2 = madvise(mr_array[i].addr, mr_count, MADV_HUGEPAGE);
            if (ret2 < 0)
                perror("madvise error");
            notify("Finished");
        }
    }
    
    if (lflag == 1) {
        for(j = 0; j < no_loops; j++) {
            for_each_mr (i) {
                dirty_pages(NUM_PUSHBACK ,write);
		print_pages(NUM_PUSHBACK);
            }
            write++;
            sleep(rest);
        }

        notify("Finished");
    }
    
    if (pflag == 1) {
        printf("[0] initialize %d: ", mvm_id);
        notify("");
        fd = init_mvm(sz, mem, conf, mvm_id);
        if (fd < 0) {
            fprintf(stderr, "can't open /dev/heca\n");
            return;
        }

        notify("[3] dirty pages:");
        c = (mvm_id % 2)? 'd' : 'e';
        dirty_pages(NUM_PUSHBACK, c);

        notify("[6] dirty and print pages (2):");
        dirty_pages(NUM_PUSHBACK, '2');
        print_pages(NUM_PUSHBACK);

        notify("[.]disconnect:\n");
        heca_close(fd);
    }
}

static void print_usage(void)
{
 printf("usage:\n"
            "{compute:} ./tst -m (master) -f [config file name] -k (Kernel Samepage Merging command) -t (Transparent Huge Page command) -p (Pushing and Pulling command) -l [number of loops] -r [how long to rest]\n"
            "{provide:} ./tst -s (slave) -f [config file name] -i [id] -k (Kernel Samepage Merging command) -t (Transparent Huge Page command) -p (Pushing and Pulling command) -l [number of loops] -r [how long to rest]\n");
}

int main(int argc, char **argv)
{
    int mvmid = NULL, kflag = 0, tflag = 0, mflag = 0, sflag = 0, idflag = 0, pflag = 0, lflag = 0, c, no_loops, rest = 0;
    char *config, *id = NULL;
    opterr = 0;
    FILE *fp;
    size_t len = 0;
    ssize_t read;

    while ((c = getopt (argc, argv, "msf:i:pl:ktr:")) != -1) {
        switch (c) {
           case 'm':
             mflag = 1;
             break;
           case 's':
             sflag = 1;
	     break;
           case 'f':
             config = optarg;
             break;
           case 'i':
             mvmid = atoi(optarg);
             break;
           case 'p':
             pflag = 1;
             break;
           case 'l':
             no_loops = atoi(optarg);
             lflag = 1;
             break;
           case 'k':
             kflag = 1;
             break;
           case 't':
             tflag = 1;
             break;
           case 'r':
             rest = atoi(optarg);
             break;
           case '?':
             if (optopt == 'c') {
               print_usage();
               goto out;
               }
             else if (isprint (optopt)) {
               print_usage();
               goto out;
               }
             else {
               print_usage();
               goto out;
               }
             return 1;
           default:
             abort();
           }
        }

    if (((mflag == 0) && (sflag == 0)) || ((sflag == 1) && (mvmid == 0)) || ((kflag == 0) && (tflag == 0) && (pflag == 0) && (lflag == 0)) || ((lflag == 1) && (rest == 0))) {
        print_usage();
        goto out;
    }

    /* compute machine */
    if (mflag == 1)
        compute(config, kflag, tflag, pflag, lflag, no_loops, rest);

    /* provider machine */
    else if (sflag == 1) {
        fp = fopen(config, "rt");
	if (fp == NULL) {
	    printf("File does not exist\n");
	    goto out;
        }
        int i, initnum;
        char initchar;

        while((read = getline(&id, &len, fp)) != -1) {
           initchar = id[0];
      
           if ('0' <= initchar && initchar <= '9') {
               initnum = initchar - '0';
           }

           if(initnum == mvmid) {
               idflag = 1;
               break;
           }
        }

        if (idflag == 1)
           provide(config, mvmid, argv[2][0], kflag, tflag, pflag, lflag, no_loops, rest);
    }

out:
    return 0;
}

