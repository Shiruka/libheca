#include "headers/tst.h"

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
    struct unmap_data data;
    int i, j, k;

    data.dsm_id = 1;
    for_each_mr (k) {
        for (i = 0; i < n; i++) {
            data.addr = mr_array[k].addr + (PAGE_SIZE*i);
            j = ioctl(fd, DSM_TRY_PUSH_BACK_PAGE, &data);
            if (j)
                printf("%d:%d error pushing: %d\n", k, i--, j);
        }
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

    notify("[3] dirty pages:");
    c = (mvm_id % 2)? '/' : '\\';
    dirty_pages(NUM_PUSHBACK, c);

    notify("[6] dirty and print pages (2):");
    dirty_pages(NUM_PUSHBACK, '2');
    print_pages(NUM_PUSHBACK);

    notify("[.]disconnect:\n");
    dsm_cleanup(fd);
}

static void compute(char *conf_name)
{
    int fd, i;
    struct CONF *conf;

    conf = config_parse(conf_name);
    assert(conf);

    /* alloc mem and connect */
    notify("[0] initialize: ");
    for_each_mr (i)
        mr_array[i].addr = valloc(PAGE_SIZE*NUM_PAGES);
    fd = init_cvm(conf, mr_array, mr_count);

    /* test */
    notify("[1] pull all pages: ");
    print_pages(NUM_PAGES);

    notify("[2] push back pages:");
    push_pages(fd, NUM_PUSHBACK);

    notify("[4] re-pull pages:");
    print_pages(NUM_PUSHBACK);

    notify("[5] dirty and print pages (1):");
    dirty_pages(NUM_PUSHBACK, '1');
    print_pages(NUM_PUSHBACK);

    notify("[7] dirty and print pages (3):");
    dirty_pages(NUM_PUSHBACK, '3');
    print_pages(NUM_PUSHBACK);

    /* cleanup */
    notify("[.] disconnect:\n");
    dsm_cleanup(fd);
    config_clean(conf);
    for_each_mr (i)
        free(mr_array[i].addr);
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

