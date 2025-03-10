#include "sort_eff.h"

int main(int argc, char *argv[])
{
    read_test_cases();
    WAIT_FOR_PERF();

    start_tmr();
    test_q_sort();
    stop_tmr("q_sort took ");

    return 0;
}
