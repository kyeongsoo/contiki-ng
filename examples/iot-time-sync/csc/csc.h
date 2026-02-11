#ifndef CSC_H
#define CSC_H

int64_t csc_dp_div(int64_t i, int64_t D, int64_t A, int *p_num_iter);
int64_t csc_sp_div(int64_t i, int64_t D, int64_t A, int *p_num_iter);
// static uint64_t csc_ds(uint64_t i, uint64_t D, uint64_t A, int *p_num_iter);
int64_t csc_ds(int64_t i, int64_t D, int64_t A, int *p_num_iter);

#endif