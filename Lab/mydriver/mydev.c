#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
MODULE_LICENCE("GPL");

static int my_init(void){
	printk("call init\n");
	return 0;
}

static void my_exit(void){
	printk("call exit\n");
}

module_init(my_init);
module_exit(my_exit);
