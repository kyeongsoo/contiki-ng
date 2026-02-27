#ifndef CSC_DATA_H
#define CSC_DATA_H

typedef struct {
    uint32_t seq_num;
    rtimer_clock_t timestamp; // 16/32-bit timestamp (controlled by RTIMER_EXT definition)
} csc_data_t;
    
#endif
