#ifndef CSC_H_
#define CSC_H_

#if CSC_INT_SIZE == 4
typedef int32_t csc_int_t;
#define CSC_INT_PRI PRId32
#elif CSC_INT_SIZE == 8
typedef int64_t csc_int_t;
#define CSC_INT_PRI PRId64
#else
#error Unsupported CSC_INT_SIZE
#endif

csc_int_t csc_dp_div(const csc_int_t i, const csc_int_t D, const csc_int_t A, uint16_t *p_num_iter);
csc_int_t csc_sp_div(const csc_int_t i, const csc_int_t D, const csc_int_t A, uint16_t *p_num_iter);
csc_int_t csc_ds(const csc_int_t i, const csc_int_t D, const csc_int_t A, uint16_t *p_num_iter);

#endif /* CSC_H_ */
