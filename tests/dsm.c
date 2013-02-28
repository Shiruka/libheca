#include "dsm.h"

static void parse_svm_data(char *s, struct svm_data *svm)
{
    char *ptr = strstr(s, ":");
    
    *ptr = 0;

    svm->server.sin_family = AF_INET;
    svm->server.sin_addr.s_addr = inet_addr(s);
    svm->server.sin_port = htons(atoi(ptr+1));
}

/* parse up to three svm data and pass to dsm */
int init_cvm(pid_t child, struct CONF *conf, struct unmap_data *mr_array,
        int mr_count)
{
    int i, j, k, svm_count, svm_ids[3];
    char *svm_config[3];
    struct svm_data svm_array[3];

    svm_count = config_get_ints(conf, svm_ids, svm_config, 3);
    for (i = 0, k =0; i < svm_count; i++) {
        bzero(&svm_array[i], sizeof svm_array[i]);
        svm_array[i].dsm_id = 1;
        svm_array[i].svm_id = svm_ids[i];
        svm_array[i].pid = child;
        parse_svm_data(svm_config[i], &svm_array[i]);
        if (svm_array[i].svm_id != COMPUTE_ID) {
            for (j = 0; j < mr_count; j++) {
                mr_array[j].svm_ids[k] = svm_array[i].svm_id;
                mr_array[j].pid = child;
            }
            k++;
        }
    }

    return heca_master_open(svm_count, svm_array, mr_count, mr_array);
}

/* prepare a master address and connect to it */
int init_mvm(unsigned long sz, void *mem, struct CONF *conf, int mvm_id)
{
    struct sockaddr_in master_addr;
    char *cvm_data;
    struct svm_data cvm;

    bzero(&cvm, sizeof cvm);

    cvm_data = config_get(conf, COMPUTE_ID_STR);
    assert(cvm_data);
    parse_svm_data(cvm_data, &cvm);

    master_addr.sin_family = AF_INET;
    master_addr.sin_port = htons(4445);
    master_addr.sin_addr.s_addr = cvm.server.sin_addr.s_addr;

    return heca_client_open(mem, sz, mvm_id, &master_addr);
}

