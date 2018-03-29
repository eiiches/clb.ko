#ifndef _MODULE_SYSCALL_H_
#define _MODULE_SYSCALL_H_


#include <linux/init.h>


extern int __init clb_module_syscall_init(void);

extern int clb_module_syscall_exit(void);


#endif /* _MODULE_SYSCALL_H_ */
