#include <sys/types.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
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

static void compute(char *conf_name)
{
    int fd = -1, i, lockfd = -1, rc;
    struct CONF *conf = NULL;
    pid_t child;
    const char *lockfn = "/tmp/tst.lock";

    fprintf(stderr, "Parent (%d): started\n", getpid());

    for_each_mr (i) {
        mr_array[i].addr = valloc(PAGE_SIZE*NUM_PAGES);
#if 0
        mr_array[i].flags |= UD_COPY_ON_ACCESS;
#endif
    }

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
    fd = init_cvm(child, conf, mr_array, mr_count);
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

    sleep(1);

    if (flock(lockfd, LOCK_EX) < 0) {
        perror("flock");
        goto failure;
    }

    heca_close(fd);

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

    fprintf(stderr, "Parent (%d): ended\n", getpid());
    rc = 0;
    goto done;

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

static void provide(char *conf_name, int mvm_id, char c)
{
    struct CONF *conf;
    unsigned long sz = PAGE_SIZE*NUM_PAGES*mr_count;
    int fd, i;
    void *mem;

    conf = config_parse(conf_name);
    assert(conf);

    mem = valloc(sz);
    assert(mem);
    memset(mem, c, sz);
    for_each_mr (i)
        mr_array[i].addr = mem + (i * mr_array[i].sz);

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

static void print_usage(void)
{
    printf("usage:\n"
            "{compute:} ./tst [config file name] \n"
            "{provide:} ./tst [config file name] [id]\n");
}

int main(int argc, char **argv)
{
    if (argc != 2 && argc != 3) {
        print_usage();
        goto out;
    }

    /* compute machine */
    if (argc == 2) {
        compute(argv[1]);

    /* provider machine */
    } else {
        if (!atoi(argv[2])) {
            print_usage();
            goto out;
        }
        provide(argv[1], atoi(argv[2]), argv[2][0]);
    }

out:
    return 0;
}

