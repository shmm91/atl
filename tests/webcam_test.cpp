#ifndef MU_PRINT
  #define MU_PRINT 1
#endif

#include "munit.h"
#include "webcam.hpp"


void test_suite()
{
    webcam_run();
}

mu_run_tests(test_suite)