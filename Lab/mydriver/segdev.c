#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
MODULE_LICENCE("GPL");

static char msg_buffer[16]
// File Operations
static ssize_t my_read(struct file *fp, char *buf, size_t count, loff_t *fpos){
	printk("call read\n");
	return copy_to_user(buf, msg_buffer, count);
}

static ssize_t my_write(struct file *fp, const char *buf, size_t count, loff_t *fpos){
	printk("call write\n");
	return copy_from_user(msg_buffer, buf, count);
}

static int my_open(struct inode *inode, struct file *fp){
	printk("call open\n");
	return 0;
}

struct file_operations my_fops = {
	read: my_read,
	write: my_write,
	open: my_open
};

#define MAJOR_NUM 245
#define DEVICE_NAME "my_dev"

static int my_init(void){
	printk("call init\n");
	if(register_chrdev(MAJOR_NUM, DEVICE_NAME, &my_fops) < 0){
		printk("Can not get major %d\n", MAJOR_NUM);
		return(-EBUSY);
	}
	printk("16 Segment device is started and the major is %d\n", MAJOR_NUM);
	return 0;
}

static void my_exit(void){
	unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
	printk("call exit\n");
}

module_init(my_init);
module_exit(my_exit);
