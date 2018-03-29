#include "libprotobuf-c.h"
static inline void assert(int cond) {
    // no assert
}
static inline void *malloc(unsigned long size) {
    return kzalloc(size, GFP_KERNEL);
}
static inline void free(void *ptr) {
    kfree(ptr);
}
bool __eqsf2(float a, float b) {
    return a == b;
}
bool __eqdf2(double a, double b) {
    return a == b;
}
#include "protobuf-c/protobuf-c/protobuf-c.c"
