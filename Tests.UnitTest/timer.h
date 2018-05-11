#pragma once

#include <time.h>       /* clock_t, clock, CLOCKS_PER_SEC */

// Record the execution time of some code, in milliseconds.
#define DECLARE_TIMING(s)  long timeStart_##s; double timeDiff_##s; double timeTally_##s = 0; int countTally_##s = 0
#define START_TIMING(s)    timeStart_##s = clock()
#define STOP_TIMING(s)     timeDiff_##s = (double)(clock() - timeStart_##s); timeTally_##s += timeDiff_##s; countTally_##s++
#define GET_TIMING(s)      (double)(timeDiff_##s / (CLOCKS_PER_SEC))
#define GET_TOTAL_TIME(s)  timeTally_##s / (CLOCKS_PER_SEC)
#define GET_AVERAGE_TIMING(s)   (double)(countTally_##s ? timeTally_##s/ ((double)countTally_##s * CLOCKS_PER_SEC) : 0)
#define CLEAR_AVERAGE_TIMING(s) timeTally_##s = 0; countTally_##s = 0
