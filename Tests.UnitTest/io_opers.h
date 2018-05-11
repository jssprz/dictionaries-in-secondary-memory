#pragma once

#define DECLARE_IOPERS(s)  int reads_##s = 0; int writes_##s = 0; int reads_inc_##s = 0; int writes_inc_##s = 0;
#define INC_READS(s, count) reads_##s += count; reads_inc_##s++;
#define INC_WRITES(s, count) writes_##s += count; writes_inc_##s++;
#define GET_READ_AVERAGE(s)      double(reads_##s) / double(reads_inc_##s)
#define GET_WRITE_AVERAGE(s)      double(writes_##s) / double(writes_inc_##s)
#define GET_TOTAL_READS(s)   reads_##s
#define GET_TOTAL_WRITES(s)   writes_##s
#define CLEAR_IOPERS(s) reads_##s = 0; writes_##s = 0; reads_inc_##s = 0; writes_inc_##s = 0;
