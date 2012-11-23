#include "../headers/dsm.h"

static void parse_svm_data(char *s, struct svm_data *svm)
{
    int i;
    char *ptr = strstr(s, ":");

    svm->port = atoi(ptr+1);
    for (i = 0; i < ptr-s && i < 20; i++)
        svm->ip[i] = s[i];
    svm->ip[i] = '\0';
}

/* parse up to three svm data and pass to dsm */
int init_cvm(struct CONF *conf, struct unmap_data *mr_array, int mr_count)
{
    int i, j, k, svm_count, svm_ids[3];
    char *svm_config[3];
    struct svm_data svm_array[3];

    svm_count = config_get_ints(conf, svm_ids, svm_config, 3);
    for (i = 0, k =0; i < svm_count; i++) {
        svm_array[i].dsm_id = 1;
        svm_array[i].svm_id = svm_ids[i];
        parse_svm_data(svm_config[i], &svm_array[i]);
        if (svm_array[i].svm_id != COMPUTE_ID) {
            for (j = 0; j < mr_count; j++)
                mr_array[j].svm_ids[k] = svm_array[i].svm_id;
            k++;
        }
    }

    return dsm_master_init(svm_count, svm_array, mr_count, mr_array, AUTO_UNMAP);
}

/* prepare a master address and connect to it */
int init_mvm(unsigned long sz, void *mem, struct CONF *conf, int mvm_id)
{
    struct sockaddr_in master_addr;
    char *cvm_data;
    struct svm_data cvm;

    cvm_data = config_get(conf, COMPUTE_ID_STR);
    assert(cvm_data);
    parse_svm_data(cvm_data, &cvm);

    master_addr.sin_family = AF_INET;
    master_addr.sin_port = htons(4445);
    master_addr.sin_addr.s_addr = inet_addr(cvm.ip);

    return dsm_client_init(mem, sz, mvm_id, &master_addr, AUTO_UNMAP);
}

