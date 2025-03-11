#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "linux_listsort.h"
#include "queue.h"
#include "random.h"

#define TEST_COUNT 300
#define TEST_QUEUE_SIZE 100000

// #define SORT_EFF_DEBUG
#define SORT_EFF_MEASURE_TIME
#define SORT_EFF_WAIT_FOR_PERF

clock_t sort_eff_start, sort_eff_end;

#ifdef SORT_EFF_MEASURE_TIME
#define start_tmr() sort_eff_start = clock()
#define stop_tmr(msg)         \
    sort_eff_end = clock();   \
    printf("%s%f sec\n", msg, \
           (double) (sort_eff_end - sort_eff_start) / CLOCKS_PER_SEC)
#else
#define start_tmr()
#define stop_tmr(msg)
#endif

#ifdef SORT_EFF_WAIT_FOR_PERF
#define WAIT_FOR_PERF(t)           \
    printf("pid: %d\n", getpid()); \
    sleep(t)
#else
#define WAIT_FOR_PERF()
#endif


struct list_head test_cases[TEST_COUNT];
struct list_head test_cases_linux[TEST_COUNT];


#define MIN_RANDSTR_LEN 5
#define MAX_RANDSTR_LEN 10
static const char charset[] = "abcdefghijklmnopqrstuvwxyz";

#define q_print(head)                                        \
    do {                                                     \
        struct list_head *pos;                               \
        printf("List: [");                                   \
        list_for_each(pos, head) {                           \
            element_t *e = list_entry(pos, element_t, list); \
            if (pos->next == head)                           \
                printf("%s", e->value);                      \
            else                                             \
                printf("%s, ", e->value);                    \
        }                                                    \
        printf("]\n");                                       \
    } while (0)

static void fill_rand_string(char *buf, size_t buf_size)
{
    /* Fixed length for reading from binary */
    size_t len = MAX_RANDSTR_LEN;

    /* direct random to get same result */
    for (size_t n = 0; n < len; n++)
        buf[n] = charset[rand() % (sizeof(charset) - 1)];

    buf[len] = '\0';
}

static inline int cmp_ascend(void *priv,
                             struct list_head *a,
                             struct list_head *b)
{
    char *a_val = list_entry(a, element_t, list)->value;
    char *b_val = list_entry(b, element_t, list)->value;
    return strcmp(a_val, b_val);
}

static inline int cmp_descend(void *priv,
                              struct list_head *a,
                              struct list_head *b)
{
    char *a_val = list_entry(a, element_t, list)->value;
    char *b_val = list_entry(b, element_t, list)->value;
    return strcmp(b_val, a_val);
}

void q_sort_linux(struct list_head *head, bool descend)
{
    list_cmp_func_t cmp = descend ? cmp_descend : cmp_ascend;
    list_sort(NULL, head, cmp);
}

void prepare_test_cases()
{
    // make the test cases same every time
    srand(0);
    for (int i = 0; i < TEST_COUNT; i++) {
        INIT_LIST_HEAD(&test_cases[i]);
        INIT_LIST_HEAD(&test_cases_linux[i]);
        char *s = malloc(MAX_RANDSTR_LEN);
        if (!s) {
            fprintf(stderr, "Failed to allocate memory\n");
            return;
        }
        for (int j = 0; j < TEST_QUEUE_SIZE; j++) {
            fill_rand_string(s, MAX_RANDSTR_LEN);
            element_t *new_element = malloc(sizeof(element_t));
            element_t *new_element_linux = malloc(sizeof(element_t));
            if (!new_element || !new_element_linux) {
                fprintf(stderr, "Failed to allocate memory\n");
                return;
            }
            INIT_LIST_HEAD(&new_element->list);
            INIT_LIST_HEAD(&new_element_linux->list);
            new_element->value = strdup(s);
            new_element_linux->value = strdup(s);
            if (!new_element->value || !new_element_linux->value) {
                fprintf(stderr, "Failed to allocate memory\n");
                return;
            }
            list_add_tail(&new_element->list, &test_cases[i]);
            list_add_tail(&new_element_linux->list, &test_cases_linux[i]);
        }
#ifdef SORT_EFF_DEBUG
        printf("Test case %d is ready, with %d elements\n", i,
               q_size(&test_cases[i]));
        printf("Test case linux %d is ready, with %d elements\n", i,
               q_size(&test_cases_linux[i]));
#endif
    }
#ifdef SORT_EFF_DEBUG
    // print first 10 elements
    struct list_head *pos;
    int count = 0;
    list_for_each(pos, &test_cases[0]) {
        element_t *e = list_entry(pos, element_t, list);
        if (count < 10) {
            printf("%s\n", e->value);
            count++;
        }
    }
#endif
    /* Store the data to binary */
    FILE *fp = fopen("test_cases.bin", "wb");
    if (!fp) {
        fprintf(stderr, "Failed to open file\n");
        return;
    }
    for (int i = 0; i < TEST_COUNT; i++) {
        struct list_head *pos;
        list_for_each(pos, &test_cases[i]) {
            element_t *e = list_entry(pos, element_t, list);
            assert(strlen(e->value) == MAX_RANDSTR_LEN);
            fwrite(e->value, sizeof(char), strlen(e->value), fp);
        }
    }
    fclose(fp);
    printf("Test cases are stored in test_cases.bin\n");
}

void read_test_cases()
{
#ifdef SORT_EFF_DEBUG
    printf("Reading test cases from test_cases.bin\n");
#endif
    FILE *fp = fopen("test_cases.bin", "rb");
    int32_t cur_count = 0;
    if (!fp) {
        fprintf(stderr, "Failed to open file\n");
        return;
    }
    for (int i = 0; i < TEST_COUNT; i++) {
        INIT_LIST_HEAD(&test_cases[i]);
        INIT_LIST_HEAD(&test_cases_linux[i]);
        char *s = malloc(MAX_RANDSTR_LEN + 1);
        if (!s) {
            fprintf(stderr, "Failed to allocate memory\n");
            return;
        }
        s[MAX_RANDSTR_LEN] = '\0';
        for (int j = 0; j < TEST_QUEUE_SIZE; j++) {
            element_t *new_element = malloc(sizeof(element_t));
            element_t *new_element_linux = malloc(sizeof(element_t));
            if (!new_element || !new_element_linux) {
                fprintf(stderr, "Failed to allocate memory\n");
                return;
            }
            INIT_LIST_HEAD(&new_element->list);
            INIT_LIST_HEAD(&new_element_linux->list);
            if (fread(s, sizeof(char), MAX_RANDSTR_LEN, fp) !=
                MAX_RANDSTR_LEN) {
                printf("failed to data at cur_count = %d\n", cur_count);
                fprintf(stderr, "Failed to read file\n");
                return;
            }
            new_element->value = strdup(s);
            new_element_linux->value = strdup(s);
            if (!new_element->value || !new_element_linux->value) {
                fprintf(stderr, "Failed to allocate memory\n");
                return;
            }
            list_add_tail(&new_element->list, &test_cases[i]);
            list_add_tail(&new_element_linux->list, &test_cases_linux[i]);
            cur_count++;
        }
#ifdef SORT_EFF_DEBUG
        printf("Test case       %d is ready, with %d elements\n", i,
               q_size(&test_cases[i]));
        printf("Test case linux %d is ready, with %d elements\n", i,
               q_size(&test_cases_linux[i]));
#endif
    }
    fclose(fp);
#ifdef SORT_EFF_DEBUG
    printf("Printing first 10 elements\n");
    // print first 10 elements
    struct list_head *pos;
    int count = 0;
    list_for_each(pos, &test_cases_linux[0]) {
        element_t *e = list_entry(pos, element_t, list);
        if (count < 10) {
            printf("%s\n", e->value);
            count++;
        }
    }
#endif
}


void test_q_sort()
{
    for (int i = 0; i < TEST_COUNT; i++)
        q_sort(&test_cases[i], true);
}

void test_q_sort_linux()
{
    for (int i = 0; i < TEST_COUNT; i++)
        q_sort_linux(&test_cases_linux[i], true);
}

void free_test_cases()
{
    for (int i = 0; i < TEST_COUNT; i++) {
        q_free(&test_cases[i]);
        q_free(&test_cases_linux[i]);
    }
}
