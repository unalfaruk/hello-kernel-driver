#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#define SYS_RINGBUF_NAME	"sys_ringbuf"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("unal");
MODULE_DESCRIPTION("sys_ringbuf kernel module");
MODULE_VERSION("0.1");
static dev_t dev_num;
static struct cdev sys_ringbuf_cdev;

static int sys_ringbuf_open(struct inode *inode, struct file *filp)
{
	pr_info("sys_ringbuf: open\n");
	return 0;
}

static int sys_ringbuf_release(struct inode *inode, struct file *filp)
{
	pr_info("sys_ringbuf: release\n");
	return 0;
}

static ssize_t sys_ringbuf_read(struct file *filp, char __user *buf,
				size_t count, loff_t *ppos)
{
	pr_info("sys_ringbuf: read (count=%zu)\n", count);
	return 0; /* no data yet */
}

static ssize_t sys_ringbuf_write(struct file *filp, const char __user *buf,
				 size_t count, loff_t *ppos)
{
	pr_info("sys_ringbuf: write (count=%zu)\n", count);
	return count; /* pretend we accepted everything */
}

static const struct file_operations sys_ringbuf_fops = {
	.owner		= THIS_MODULE,
	.open		= sys_ringbuf_open,
	.release	= sys_ringbuf_release,
	.read		= sys_ringbuf_read,
	.write		= sys_ringbuf_write,
};

static int __init sys_ringbuf_init(void)
{
	int ret;

	/* Ask the kernel for one char-device number (major + minor 0). */
	ret = alloc_chrdev_region(&dev_num, 0, 1, SYS_RINGBUF_NAME);
	if (ret < 0) {
		pr_err("sys_ringbuf: alloc_chrdev_region failed (%d)\n", ret);
		return ret;
	}

	cdev_init(&sys_ringbuf_cdev, &sys_ringbuf_fops);
	sys_ringbuf_cdev.owner = THIS_MODULE;

	ret = cdev_add(&sys_ringbuf_cdev, dev_num, 1);
	if (ret < 0) {
		pr_err("sys_ringbuf: cdev_add failed (%d)\n", ret);
		unregister_chrdev_region(dev_num, 1);
		return ret;
	}

	pr_info("sys_ringbuf: registered major=%u minor=%u\n",
		MAJOR(dev_num), MINOR(dev_num));
	return 0;
}

static void __exit sys_ringbuf_exit(void)
{
	cdev_del(&sys_ringbuf_cdev);
	unregister_chrdev_region(dev_num, 1);
	pr_info("sys_ringbuf: unregistered\n");
}

module_init(sys_ringbuf_init);
module_exit(sys_ringbuf_exit);
