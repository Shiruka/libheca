#include "dsm.h"

static void parse_hproc_data(char *s, struct hecaioc_hproc *hproc)
{
        char *ptr = strstr(s, ":");

        *ptr = 0;

        hproc->remote.sin_family = AF_INET;
        hproc->remote.sin_addr.s_addr = inet_addr(s);
        hproc->remote.sin_port = htons(atoi(ptr+1));
}

/* parse up to three hproc data and pass to hspace */
int init_cvm(pid_t child, struct CONF *conf, struct hecaioc_hmr *mr_array,
                int mr_count, int populate_mrs)
{
        int i, j, k, hproc_count, hproc_ids[3];
        char *hproc_config[3];
        struct hecaioc_hproc hproc_array[3];

        hproc_count = config_get_ints(conf, hproc_ids, hproc_config, 3);
        for (i = 0, k = 0; i < hproc_count; i++) {
                bzero(&hproc_array[i], sizeof hproc_array[i]);
                hproc_array[i].hspace_id = 1;
                hproc_array[i].hproc_id = hproc_ids[i];
                hproc_array[i].pid = child;
                parse_hproc_data(hproc_config[i], &hproc_array[i]);
                if (populate_mrs && hproc_array[i].hproc_id != COMPUTE_ID) {
                        for (j = 0; j < mr_count; j++)
                                mr_array[j].hproc_ids[k] = hproc_array[i].hproc_id;
                        k++;
                }
        }

        return heca_master_open(hproc_count, hproc_array, mr_count, mr_array);
}

/* prepare a master address and connect to it */
int init_mvm(unsigned long sz, void *mem, struct CONF *conf, int mvm_id)
{
        struct sockaddr_in master_addr;
        char *cvm_data;
        struct hecaioc_hproc cvm;

        bzero(&cvm, sizeof cvm);

        cvm_data = config_get(conf, COMPUTE_ID_STR);
        assert(cvm_data);
        parse_hproc_data(cvm_data, &cvm);

        master_addr.sin_family = AF_INET;
        master_addr.sin_port = htons(4445);
        master_addr.sin_addr.s_addr = cvm.remote.sin_addr.s_addr;

        return heca_client_open(mem, sz, mvm_id, &master_addr);
}

