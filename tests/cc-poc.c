#include "cc-poc.h"

/*
 * TODO:
 *  - move imdb to lib
 *  - real testing (disjoint, joint, write...)
 */
static int imdb(void *mem, unsigned long sz, int num)
{
        int i;
        struct timeval start, end;

        printf("[1] reading all mem...\n");
        gettimeofday(&start, NULL);
        for (i = 0; i < sz/sizeof(int); i++) {
                int *p = (int *) (mem + i*sizeof(int)),
                    e = ((unsigned long)p-(unsigned long)mem)/(sz/num)+1;

                if (i % 100000 == 0)
                        printf("[%d] %d\n", i, *p);

                if (*p != e) {
                        printf("[FAIL] corruption at (%lu); exp=%d, real=%d\n",
                                        (unsigned long) p, e, *p);
                        return -1;
                }
        }
        gettimeofday(&end, NULL);
        printf("[2] ... finished.\n");
        printf("elapsed: %ldms\n", (long int) (((end.tv_sec-start.tv_sec)*1000 +
                                        (end.tv_usec-start.tv_usec)/1000.0) + 0.5));

        return 0;
}

static void compute(int hproc_id, char *conf_name)
{
        struct CONF *conf = NULL;
        int fd, i, num;
        void *mem, *hproc_mem;
        unsigned long sz, hproc_sz;
        char x;

        assert(conf_name);
        conf = config_parse(conf_name);
        assert(conf);
        num = config_count_ints(conf);
        assert(num);
        sz = config_get_int(conf, "size");
        assert(sz);
        sz *= 1024 * 1024;

        /* allocate mem and init */
        mem = valloc(sz);
        assert(mem);
        hproc_sz = sz/num;
        hproc_mem = mem + (hproc_id-1)*(hproc_sz);
        for (i = 0; i < hproc_sz/sizeof(int); i++)
                *((int *) (hproc_mem + i*sizeof(int))) = hproc_id;

        /* initialize */
        if (hproc_id == 1) {
                struct hecaioc_hmr mr_array[num];

                for (i = 0; i < num; i++) {
                        mr_array[i].hmr_id = i+1;
                        mr_array[i].hspace_id = 1;
                        mr_array[i].hproc_ids[0] = i+1;
                        mr_array[i].hproc_ids[1] = 0;
                        mr_array[i].sz = hproc_sz;
                        mr_array[i].addr = mem + hproc_sz*i;
                        mr_array[i].flags = UD_AUTO_UNMAP | UD_SHARED;
                }
                fd = init_cvm(0, conf, mr_array, num, 0);

        } else {
                fd = init_mvm(sz, mem, conf, hproc_id);
        }

        /* payload */
        imdb(mem, sz, num);
        printf("Benchmark finished.\n");
        fscanf(stdin, "%c", &x);

        /* cleanup */
        heca_close(fd);
        config_clean(conf);
        free(mem);
}

static void print_usage(void)
{
        printf("usage: ./poc [config file name] [id]\n");
}

int main(int argc, char **argv)
{
        if (argc != 3 || !atoi(argv[2])) {
                print_usage();
                goto out;
        }

        compute(atoi(argv[2]), argv[1]);

out:
        return 0;
}

