#ifndef CSC_DATA_H
#define CSC_DATA_H

typedef struct {
    uint32_t seq_num;
#ifdef RTIMER_EXT
    rtimer32_clock_t timestamp; // 32-bit timestamp for extended rtimer
#else
    rtimer_clock_t timestamp; // 16-bit timestamp for standard rtimer
#endif
} csc_data_t;
    
#endif
