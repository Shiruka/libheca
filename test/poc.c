#include "headers/poc.h"

static void provide(char *conf_name, char *mvm_id_str)
{
    struct CONF *conf;
    unsigned long sz;
    int fd, mvm_id;
    void *mem;
    char x;

    conf = config_parse(conf_name);
    sz = config_get_int(conf, "size");
    sz *= 1024 * 1024;
    assert(sz);
    mvm_id = atoi(mvm_id_str);
    assert(mvm_id);

    mem = valloc(sz);
    assert(mem);

    fd = init_mvm(sz, mem, conf, mvm_id);
    printf("Providing, press [enter] to finish.\n");
    fscanf(stdin, "%c", &x);

    dsm_cleanup(fd);
    config_clean(conf);
    free(mem);
}

static void compute(unsigned long sz, char *conf_name)
{
    struct CONF *conf = NULL;
    int fd;
    void *mem;
    char x;

    if (conf_name) {
        conf = config_parse(conf_name);
        sz = config_get_int(conf, "size");
    }

    /* allocate mem and init */
    assert(sz);
    sz *= 1024 * 1024;
    mem = valloc(sz);
    assert(mem);
    if (conf) {
        struct unmap_data mr_array = {.id=1,.dsm_id=1,.sz=sz,.addr=mem};
        fd = init_cvm(conf, &mr_array, 1);
    }
    /* payload: configurable in the future */
    payload = quicksort;
    payload(mem, sz);
    printf("Benchmark finished.\n");
    fscanf(stdin, "%c", &x);

    /* cleanup */
    if (conf) {
        dsm_cleanup(fd);
        config_clean(conf);
    }
    free(mem);
}

static void print_usage(void)
{
    printf("usage:\n"
            "{compute:} ./poc [config file name]\n"
            "{provide:} ./poc [config file name] [id]\n"
            "{compute locally:} ./poc [memory size]\n");
}

int main(int argc, char **argv)
{
    if (argc != 2 && argc != 3) {
        print_usage();
        goto out;
    }

    /* compute machine */
    if (argc == 2) {

        /* local bench configuration */
        if (atoi(argv[1]))
            compute(atoi(argv[1]), NULL);

        /* dsm configuration */
        else
            compute(0, argv[1]);

    /* provider machine */
    } else {
        provide(argv[1], argv[2]);
    }

out:
    return 0;
}

