#include <stdio.h>

// #define KOS_DEBUG 0
#if defined(KOS_DEBUG) && (KOS_DEBUG != 0)
#define KOS_TRACE_INF(fmt, ...) fprintf(stderr, "(%s)%s:%d:" fmt "\n", \
                                        __FILE__, __func__, __LINE__, ##__VA_ARGS__)
#else
#define KOS_TRACE_INF(fmt, ...)
#endif
