#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("unal");
MODULE_DESCRIPTION("sys_ringbuf kernel module");
MODULE_VERSION("0.1");

/*
 * Step 1: file_operations — the callback table for a char device.
 *
 * When userspace later does open/read/write on /dev/sys_ringbuf, the VFS
 * dispatches to these functions. For now they only log; we have not
 * registered a device yet, so nothing calls them until the next step.
 */

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
	pr_info("sys_ringbuf: module loaded (fops defined, not registered yet)\n");
	return 0;
}

static void __exit sys_ringbuf_exit(void)
{
	pr_info("sys_ringbuf: module unloaded\n");
}

module_init(sys_ringbuf_init);
module_exit(sys_ringbuf_exit);
