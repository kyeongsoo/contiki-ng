#ifndef NULLNETDATA_H
#define NULLNETDATA_H

typedef struct {
    uint32_t seq_num;
    rtimer_clock_t timestamp;
} nullnet_data_t;
    
#endif
