#include "sort_eff.h"

void print_usage(char *prog_name)
{
    fprintf(stderr, "Usage: %s [linux, qsort, read] [-wait]\n", prog_name);
    fprintf(stderr, "\t-linux: Test linux sort\n");
    fprintf(stderr, "\t-qsort: Test qsort\n");
    fprintf(stderr, "\t-read: Test test cases\n");
    fprintf(stderr, "\t-wait: Wait 10 sec for performance analysis\n");
}

static bool inline run_test(const char *method, int wait_time)
{
    if (!strcmp(method, "read")) {
        WAIT_FOR_PERF(wait_time);
        start_tmr();
        read_test_cases();
        stop_tmr("Read testcases took ");
    } else if (!strcmp(method, "qsort")) {
        read_test_cases();
        WAIT_FOR_PERF(wait_time);
        start_tmr();
        test_q_sort();
        stop_tmr("q_sort sort took ");
    } else if (!strcmp(method, "linux")) {
        read_test_cases();
        WAIT_FOR_PERF(wait_time);
        start_tmr();
        test_q_sort_linux();
        stop_tmr("Linux sort took ");
    } else
        return false;

    return true;
}

int main(int argc, char *argv[])
{
    if (argc < 1) {
        print_usage(argv[0]);
        return 1;
    }

    int wait_time = 0;
    const char *method = NULL;

    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-wait") == 0) {
            wait_time = 10;
        } else if (!method) {
            method = argv[i];  // First non-option argument is the method
        } else {
            print_usage(argv[0]);
            return 1;
        }
    }

    if (!method) {
        print_usage(argv[0]);
        return 1;
    }

    if (!run_test(method, wait_time)) {
        print_usage(argv[0]);
        return 1;
    }

    return 0;
}
