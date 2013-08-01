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

static void push_pages(int fd, unsigned long n)
{
    struct hecaioc_ps data;
    int k;

    bzero(&data, sizeof data);
    data.sz = PAGE_SIZE * n;
    data.pid = 0;
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
    int fd = -1, i;
    struct CONF *conf;

    conf = config_parse(conf_name);
    assert(conf);

    for_each_mr (i) {
        mr_array[i].addr = valloc(PAGE_SIZE*NUM_PAGES);
        mr_array[i].flags |= UD_SHARED;
    }

    notify("[0] initialize cvm:\n");
    fd = init_cvm(0, conf, mr_array, mr_count, 1);
    if (fd < 0) {
        fprintf(stderr, "can't open /dev/heca\n");
        goto out;
    }

    notify("[1] pull all pages: (read mode, should show '2')");
    print_pages(NUM_PAGES);

    notify("[2] push back pages: (should just be discarded)");
    push_pages(fd, NUM_PUSHBACK);

    notify("[4] re-pull pages: (read mode, should show 'e')");
    print_pages(NUM_PUSHBACK);

    notify("[5] dirty and print pages: (acquiring write, writing '1', printing)");
    dirty_pages(NUM_PUSHBACK, '1');
    print_pages(NUM_PUSHBACK);

    notify("[8] print pages: (getting read from mvm, should show '2')");
    print_pages(NUM_PUSHBACK);

    notify("[9] dirty and print pages: (acquiring write, writing '1', printing)");
    dirty_pages(NUM_PUSHBACK, '1');
    print_pages(NUM_PUSHBACK);

    notify("[11] dirty and print pages: (acquiring write, writing '1', printing)");
    dirty_pages(NUM_PUSHBACK, '1');
    print_pages(NUM_PUSHBACK);

    notify("[.] disconnect:\n");
    heca_close(fd);

out:
    config_clean(conf);
    for_each_mr (i) {
        free(mr_array[i].addr);
        mr_array[i].addr = 0;
    }
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

    notify("[3] dirty pages: (writing locally 'e')");
    c = (mvm_id % 2)? 'd' : 'e';
    dirty_pages(NUM_PUSHBACK, c);

    notify("[6] print pages: (getting read from cvm, should show '1')");
    print_pages(NUM_PUSHBACK);

    notify("[7] dirty and print pages: (acquiring write, writing '2', printing)");
    dirty_pages(NUM_PUSHBACK, '2');
    print_pages(NUM_PUSHBACK);

    notify("[10] dirty and print pages: (acquiring write, writing '2', printing)");
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

