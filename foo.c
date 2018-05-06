#include <linux/slab.h>
typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
static inline void assert(int cond) {
    // no assert
}
static inline void *malloc(unsigned long size) {
    return kzalloc(size, GFP_KERNEL);
}
static inline void free(void *ptr) {
    kfree(ptr);
}
#include "protobuf/foo.pb-c.c"

void foo_test(void) {
    Person p = PERSON__INIT;
    p.name = "test";
    p.id = 10;
    p.email = "eiichi.sato@gmail.com";

    int len = person__get_packed_size(&p);
    unsigned char *ptr = (unsigned char *) kzalloc(len, GFP_KERNEL);
    person__pack(&p, ptr);

    Person *q = person__unpack(NULL, len, ptr);
    printk("id = %d, name = %s, email = %s\n", q->id, q->name, q->email);
    person__free_unpacked(q, NULL);
    kfree(ptr);
}
