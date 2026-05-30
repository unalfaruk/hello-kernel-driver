#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/uaccess.h>

#define SYS_RINGBUF_NAME	"sys_ringbuf"
#define STORAGE_SIZE		512

MODULE_LICENSE("GPL");
MODULE_AUTHOR("unal");
MODULE_DESCRIPTION("sys_ringbuf kernel module");
MODULE_VERSION("0.1");
static char storage[STORAGE_SIZE];
static size_t storage_len;

static dev_t dev_num;
static struct class *sys_ringbuf_class;
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
	size_t to_copy;
	size_t available;

	if (!buf)
		return -EINVAL;

	if (*ppos >= storage_len)
		return 0;

	available = storage_len - *ppos;
	to_copy = min(count, available);

	if (copy_to_user(buf, storage + *ppos, to_copy))
		return -EFAULT;

	*ppos += to_copy;
	pr_info("sys_ringbuf: read %zu bytes\n", to_copy);
	return to_copy;
}

static ssize_t sys_ringbuf_write(struct file *filp, const char __user *buf,
				 size_t count, loff_t *ppos)
{
	size_t to_copy;

	if (!buf)
		return -EINVAL;

	to_copy = min(count, (size_t)STORAGE_SIZE);
	if (copy_from_user(storage, buf, to_copy))
		return -EFAULT;

	storage_len = to_copy;
	pr_info("sys_ringbuf: stored %zu bytes\n", to_copy);
	return to_copy;
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
	struct device *dev;
	int ret;

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
		goto err_unregister;
	}

	/* Step 3: sysfs class + /dev node (udev may set permissions). */
	sys_ringbuf_class = class_create(SYS_RINGBUF_NAME);
	if (IS_ERR(sys_ringbuf_class)) {
		ret = PTR_ERR(sys_ringbuf_class);
		pr_err("sys_ringbuf: class_create failed (%d)\n", ret);
		goto err_cdev;
	}

	dev = device_create(sys_ringbuf_class, NULL, dev_num, NULL,
			    SYS_RINGBUF_NAME);
	if (IS_ERR(dev)) {
		ret = PTR_ERR(dev);
		pr_err("sys_ringbuf: device_create failed (%d)\n", ret);
		goto err_class;
	}

	pr_info("sys_ringbuf: ready at /dev/%s (major=%u minor=%u)\n",
		SYS_RINGBUF_NAME, MAJOR(dev_num), MINOR(dev_num));
	return 0;

err_class:
	class_destroy(sys_ringbuf_class);
	sys_ringbuf_class = NULL;
err_cdev:
	cdev_del(&sys_ringbuf_cdev);
err_unregister:
	unregister_chrdev_region(dev_num, 1);
	return ret;
}

static void __exit sys_ringbuf_exit(void)
{
	device_destroy(sys_ringbuf_class, dev_num);
	class_destroy(sys_ringbuf_class);
	cdev_del(&sys_ringbuf_cdev);
	unregister_chrdev_region(dev_num, 1);
	pr_info("sys_ringbuf: unregistered\n");
}

module_init(sys_ringbuf_init);
module_exit(sys_ringbuf_exit);
