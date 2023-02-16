#include<linux/module.h>
#include<linux/init.h>
#include<linux/kernel.h>
#include<linux/fs.h>
#include<linux/uaccess.h>

#define MAJOR_NUM 244
#define DEV_NAME "mydev"

MODULE_LICENSE("GPL");

#define BUF_LEN 80
static char msg_buf[BUF_LEN];

// File operations
static ssize_t seg_open(struct inode *inode, struct file *fp){
  printk("seg opened\n");
  return 0;
}

static ssize_t seg_read(struct file* fp, char* buf, size_t count, loff_t* fpos){
  // printk("call read\n");
  
  return copy_to_user(buf, msg_buf, count);
}

static ssize_t seg_write(struct file* fp, const char* buf, size_t count, loff_t* fpos){
  // printk("call write\n");
  
  return copy_from_user(msg_buf, buf, count);
}

static struct file_operations fops = {
  read: seg_read,
  write: seg_write,
  open: seg_open
};

static int seg_init(void){
  if(register_chrdev(MAJOR_NUM, DEV_NAME, &fops) < 0){
    printk("Register failed");
    return -1;
  }

  printk("driver init major number %d\n", MAJOR_NUM);
  return 0;
}

static void seg_exit(void){
  printk("driver exit");
}

module_init(seg_init);
module_exit(seg_exit);
