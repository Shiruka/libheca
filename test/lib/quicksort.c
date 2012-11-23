#include "../headers/quicksort.h"

static unsigned int range = 20;
static unsigned int num_cpus;
static unsigned int cpu = 0;
static pthread_mutex_t mutex;

static inline void generate(void *mem, unsigned long len)
{
    unsigned long i, j = 0;

    arr = (struct chunk *)mem;
    for (i = 0; i < len; i++)
        set_chunk(&arr[i], j++ % range);
}

static inline unsigned long cpivot(unsigned long left, unsigned long right)
{
    unsigned long med = (right-left)/2;

    if (cmp(&arr[left], &arr[right])) {
        if (cmp(&arr[right], &arr[med]))
            return right;
        if (cmp(&arr[left], &arr[med]))
            return med;
        return left;
    } else {
        if (cmp(&arr[left], &arr[med]))
            return left;
        if (cmp(&arr[right], &arr[med]))
            return med;
        return right;
    }
}

static inline unsigned long partition(unsigned long left, unsigned long right)
{
    unsigned long i = left+1, j = right;
    struct chunk *pivot;

    swap(left, cpivot(left, right));
    pivot = &arr[left];
    while (i <= j) {
        while (i <= right && !cmp(&arr[i], pivot))
            i++;
        while (j >= left && cmp(&arr[j], pivot))
            j--;
        if (i < j)
            swap(i, j);
    }
    swap(left, j);

    return j;
}

static void iterative_sort(unsigned long left, unsigned long right)
{
    unsigned long stack[(int)(log2(right-left)+1)], k = 0;

    stack[k++] = left;
    stack[k++] = right;

    while (k) {
        unsigned long left, right, i;

        /* partition */
        right = stack[--k];
        left = stack[--k];
        if (left >= right)
            continue;
        i = partition(left, right);

        /* iterate */
        stack[k++] = left;
        stack[k++] = i-1;
        stack[k++] = i+1;
        stack[k++] = right;
    }
}

static void *parallel_sort(void *ptr)
{
    struct recurse_sort_data *data = (struct recurse_sort_data *) ptr;
    unsigned long i, left = data->left, right = data->right;
    int nesting = data->nesting;

    /* partition */
    if (left >= right)
        goto out;
    i = partition(left, right);

    /* if more cpus available, parallel recurse */
    if (1 << nesting <= num_cpus) {
        struct recurse_sort_data 
            itr_left = {.left=left, .right=i-1, .nesting=nesting+1}, 
            itr_right = {.left=i+1, .right=right, .nesting=nesting+1};
        pthread_t thread_left;

        pthread_create(&thread_left, NULL, &parallel_sort, (void *) &itr_left);
        parallel_sort( (void * )&itr_right);

        pthread_join(thread_left, NULL);

    /* stay on this cpu, iterative sort */
    } else {
        cpu_set_t mask;
        int c;

        CPU_ZERO(&mask);
        pthread_mutex_lock(&mutex);
        CPU_SET((int)cpu++, &mask);
        pthread_mutex_unlock(&mutex);
        pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask);

        for (c = 0; c < num_cpus; c++) {
            if (CPU_ISSET(c, &mask)) {
                printf(" ... > thread set on %d / %d (nesting=%d)\n", c,
                        num_cpus, nesting);
            }
        }

        iterative_sort(left, i-1);
        iterative_sort(i+1, right);
    }

out:
    return NULL; /* pthread compatible */
}

static void sort(unsigned long len)
{
    struct recurse_sort_data data = {.left=0, .right=len-1, .nesting=0};
    pthread_mutex_init(&mutex, NULL);
    parallel_sort((void *) &data);
    pthread_mutex_destroy(&mutex);
}

int quicksort(void *mem, unsigned long sz)
{
    unsigned long i, len = sz / sizeof(struct chunk);
    size_t start, end;

    num_cpus = sysconf(_SC_NPROCESSORS_ONLN);

    printf("generating %lu elements ...\n", len);
    generate(mem, len);
    printf(" ... total %luMB allocated\n", sz / 1024 / 1024);

    printf("---- bench start ----\n");
    start = time(NULL);
    sort(len);
    end = time(NULL);
    printf("---- bench end ----\n");
    printf("total %lu seconds ... \n", end - start);

    for (i = len - 1; i > 0 && !cmp(&arr[i-1], &arr[i]); i--)
        ;
    if (i) {
        printf(" ... payload failure\n");
        return -1;
    }

    printf(" ... payload ok\n");
    return 0;
}

