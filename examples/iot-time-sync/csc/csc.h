#ifndef CSC_H_
#define CSC_H_

#if CSC_INT_SIZE == 4
typedef int32_t csc_int_t;
#define CSC_INT_MAX INT32_MAX
#define CSC_INT_MIN INT32_MIN
#define CSC_INT_PRI PRId32
#elif CSC_INT_SIZE == 8
typedef int64_t csc_int_t;
#define CSC_INT_MAX INT64_MAX
#define CSC_INT_MIN INT64_MIN
#define CSC_INT_PRI PRId64
#elif CSC_INT_SIZE == 16 && defined(__SIZEOF_INT128__)
typedef __int128_t csc_int_t;
#define CSC_INT_MAX ((__int128_t)(( (__uint128_t)1 << 127 ) - 1))
#define CSC_INT_MIN (-CSC_INT_MAX - 1)
#define CSC_INT_PRI PRId128
#else
#error Unsupported CSC_INT_SIZE
#endif

// type-independent implementation
#ifndef ABS
#define ABS(x) (((x) < 0) ? -(x) : (x))
#endif

csc_int_t csc_dp(const csc_int_t i, const csc_int_t D, const csc_int_t A, uint32_t *p_num_iter);
csc_int_t csc_sp(const csc_int_t i, const csc_int_t D, const csc_int_t A, uint32_t *p_num_iter);
csc_int_t csc_ds(const csc_int_t i, const csc_int_t D, const csc_int_t A, uint32_t *p_num_iter);
csc_int_t csc_ds2(const csc_int_t i, const csc_int_t D, const csc_int_t A, uint32_t *p_num_iter);
csc_int_t csc_ds3(const csc_int_t i, const csc_int_t D, const csc_int_t A, uint32_t *p_num_iter);
csc_int_t csc_eds(const csc_int_t i, const csc_int_t D, const csc_int_t A, uint32_t *p_num_iter);

#endif /* CSC_H_ */
