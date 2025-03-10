#include "sort_eff.h"

int main(int argc, char *argv[])
{
    // prepare_test_cases();
    WAIT_FOR_PERF();

    start_tmr();
    read_test_cases();
    stop_tmr("Read testcases took ");

    return 0;
}
