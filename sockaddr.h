#ifndef _SOCKADDR_H_
#define _SOCKADDR_H_


#ifdef CLB_PRIVATE


#include <linux/socket.h> // struct sockaddr_storage;


extern unsigned long sockaddr_hash(struct sockaddr_storage *addr);

extern bool sockaddr_equals(struct sockaddr_storage *a, struct sockaddr_storage *b);


#endif /* CLB_PRIVATE */


#endif /* _SOCKADDR_H_ */
