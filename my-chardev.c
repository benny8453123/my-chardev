#include <linux/module.h>
/*Using for struct file / file operation ops */
#include <linux/fs.h>
#include <linux/uaccess.h>
/* Using for char device */
#include <linux/cdev.h>
/* Using for kmalloc/kfree */
#include <linux/slab.h>
/* Using for memset */
#include <linux/string.h>

#include "my-chardev.h"

#define CDEV_NAME "my_char_dev"

/* For FIFO buf size*/
#define MAX_FIFO_BUF_SIZE 64

static dev_t dev;
static struct cdev *my_cdev = NULL;
static signed count = 1;

#ifdef USE_KFIFO
#include <linux/kfifo.h>
DEFINE_KFIFO(fifo_buffer, char, MAX_FIFO_BUF_SIZE);
#else
/* For virtual  FIFO buf address */
static char *fifo_buffer = NULL;
#endif /* USE_KFIFO */


static int my_chardev_open(struct inode *inode, struct file *file)
{
	int major = MAJOR(inode->i_rdev);
	int minor = MINOR(inode->i_rdev);
	
	my_chardev_info("Enter func:%s\n", __func__);	


	my_chardev_info("%s: major=%d, minor=%d\n", __func__, major, minor);	
	return 0;
}


static int my_chardev_release(struct inode *inode, struct file *file)
{
	my_chardev_info("Enter func:%s\n", __func__);	
	return 0;
}

static ssize_t my_chardev_read(struct file *file, char __user *buf, size_t lbuf, loff_t *ppos)
{
	int ret = -1;
	int actual_read_count;
#ifndef USE_KFIFO
	int need_read_count;
	int max_free_size;
#endif /* USE_KFIFO */

	my_chardev_info("Enter func:%s\n", __func__);	

#ifdef USE_KFIFO
	ret = kfifo_to_user(&fifo_buffer, buf, lbuf, &actual_read_count);
	if (ret)
		return -EIO;
#else
	my_chardev_info("%s: fifo_buffer = %s, ppos = %lld\n", __func__, fifo_buffer, *ppos);

	max_free_size = MAX_FIFO_BUF_SIZE - *ppos;
	need_read_count = max_free_size > lbuf ? lbuf : max_free_size;
	if (need_read_count == 0)
		my_chardev_err("Error, no space for read");

	ret = copy_to_user(buf, fifo_buffer + *ppos, need_read_count);
	if (ret == need_read_count)
		return -EFAULT;

	actual_read_count = need_read_count - ret;
	*ppos += actual_read_count;
#endif /* USE_KFIFO */

	my_chardev_info("%s: actual_read_count = %d, ppos = %lld\n", __func__, actual_read_count, *ppos);

	return actual_read_count;
}

static ssize_t my_chardev_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	int ret = -1;
	int actual_write_count;
#ifndef USE_KFIFO
	int need_write_count;
	int free_space_size;
#endif /* USE_KFIFO */

	my_chardev_info("Enter func:%s\n", __func__);	
	my_chardev_info("ppos = %lld\n", *ppos);

#ifdef USE_KFIFO
	ret = kfifo_from_user(&fifo_buffer, buf, count, &actual_write_count);
	if (ret)
		return -EIO;
#else	
	free_space_size = MAX_FIFO_BUF_SIZE - *ppos;
	need_write_count = free_space_size > count ? count : free_space_size;
	if (need_write_count == 0)
		my_chardev_err("Error, no space for write");

	ret = copy_from_user(fifo_buffer + *ppos, buf, need_write_count);
	if (ret == need_write_count)
	       return -EFAULT;

	actual_write_count = need_write_count - ret;
	*ppos += actual_write_count;
	
	my_chardev_info("%s: fifo_buffer = %s\n", __func__, fifo_buffer);
#endif /* USE_KFIFO */

	my_chardev_info("%s: actual_write_count = %d, ppos = %lld\n", __func__, actual_write_count, *ppos);

	return actual_write_count;
}

static const struct file_operations my_chardev_fops = {
	.owner = THIS_MODULE,
	.open = my_chardev_open,
	.release = my_chardev_release,
	.read = my_chardev_read,
	.write = my_chardev_write,
};

static int __init my_chardev_init(void)
{
	int ret = -1;

	ret = alloc_chrdev_region(&dev, 0, count, CDEV_NAME);
	if (ret) {
		my_chardev_err("Fail to alloc char device region");
		goto out; 
	}

	my_cdev = cdev_alloc();
	if (!my_cdev) {
		ret = -1;
		my_chardev_err("Fail to alloc char device");
		goto release_chrdev_region; 
	}

	cdev_init(my_cdev, &my_chardev_fops);
	
	ret = cdev_add(my_cdev, dev, count);
	if (ret) {
		my_chardev_err("cdev_add failed");
		goto release_chardev;
	}

	my_chardev_info("char device regist successful: %s\n", CDEV_NAME);
	my_chardev_info("%s: major=%d, minor=%d\n", __func__, MAJOR(dev), MINOR(dev));	

#ifdef USE_KFIFO
	my_chardev_info("%s: Use KFIFO\n", __func__);
#else
	my_chardev_info("%s: Use kmalloc for fifo\n", __func__);
	/* kmalloc virtual fifo buffer */
	fifo_buffer = kmalloc(MAX_FIFO_BUF_SIZE, GFP_KERNEL);
	if (!fifo_buffer) {
		ret = -1;
		my_chardev_err("Fail to alloc fifo buffer\n");
		goto release_chardev; 
	}
	memset(fifo_buffer, 0, MAX_FIFO_BUF_SIZE);
#endif /* USE_KFIFO */

	ret = 0;
	goto out;

release_chardev:
	cdev_del(my_cdev);

release_chrdev_region:
	unregister_chrdev_region(dev, count);

out:
	return ret;
}

static void __exit my_chardev_exit(void)
{
	my_chardev_info("Removing device: %s\n", CDEV_NAME);

#ifndef USE_KFIFO
	/* free fifo buffer */
	kfree(fifo_buffer);
#endif /* USE_KFIFO */

	if (my_cdev)
		cdev_del(my_cdev);

	unregister_chrdev_region(dev, count);
}

module_init(my_chardev_init);
module_exit(my_chardev_exit);

MODULE_LICENSE("GPLv2");
MODULE_AUTHOR("Ben Chang");
MODULE_DESCRIPTION("Testing for charactor device");
