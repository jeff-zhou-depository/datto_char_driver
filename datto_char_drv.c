#include <linux/module.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define CHAR_DEV_MAJOR 0
#define IOCTL_SET_VALUE 1
#define DEV_NAME "datto_char"
#define DEV_CLASS "datto_char_class"
#define DEV_CHAR_REGION "datto_char_region"
#define DATA_BUF_SIZE 1024

static int major = 0;
static int max_nr_devs = 1;
static const int min_minor = 0;
static unsigned int dev_opened = 0;
static struct rw_semaphore rw_sem;
static struct semaphore dev_sem;

static struct cdev* c_dev = NULL;
static struct class* char_dev_class = NULL;

static char ioctl_char = 0;
static char data[DATA_BUF_SIZE];

static int char_drv_open(struct inode* ind, struct file *fp) {
    if (down_interruptible(&dev_sem)) {
        return -ERESTARTSYS;
    }

    dev_opened++;
    up(&dev_sem);

    printk(KERN_INFO "datto char open count: %d", dev_opened);
    return 0;
}


static int char_drv_release(struct inode* ind, struct file *fp) {
    if (down_interruptible(&dev_sem)) {
        return -ERESTARTSYS;
    }

    dev_opened--;
    up(&dev_sem);
    printk(KERN_INFO "datto char open count after release : %d", dev_opened);
    return 0;
}


static ssize_t char_drv_read(struct file *fp, char* buf, size_t len, loff_t* offset) {
    int ret = 0;
    char c = 0;

    if (ioctl_char == 0) {
        return -EINVAL;
    }

    down_read(&rw_sem);
    c = ioctl_char;
    up_read(&rw_sem);

    memset(data, c, DATA_BUF_SIZE);

    while (len > DATA_BUF_SIZE) {
        ret = copy_to_user(buf, data, DATA_BUF_SIZE);
        if (ret != 0) {
            return -EFAULT;;
        }
        len -= DATA_BUF_SIZE;
    }

    // Fill in the remaining buffer
    if (len > 0) {
        ret = copy_to_user(buf, data, len);
        if (ret != 0) {
            return -EFAULT;;
        }
    }

    return 0;
}


static ssize_t char_drv_write(struct file *fp, const char* buf, size_t len, loff_t* offset) {
    // do nothing, use the ioctl to change the output char 
    return 0;
}


/*
 * Displayable char according to ASCII table
 */
#define CHAR_START  32
#define CHAR_END    255
#define CHAR_DEL    127

static long char_drv_ioctl(struct file* fp, unsigned int cmd, unsigned long arg) {
    char c = 0;
    printk(KERN_INFO "datto ioctl arg %lu", arg);

    down_write(&rw_sem);
    c = (char) arg;
    up_write(&rw_sem);

    switch (cmd) {
        case IOCTL_SET_VALUE:
            if ((c > CHAR_START) && (c < CHAR_END) && (c != CHAR_DEL)) {
                ioctl_char = c;
                printk(KERN_INFO "datto ioctl set value %c", ioctl_char);
            }

            break;
        default:
            break;
    }


    return 0;
}


struct file_operations char_fops = {
    .owner = THIS_MODULE,
    .open = char_drv_open,
    .release = char_drv_release,
    .read = char_drv_read,
    .write = char_drv_write,
    .unlocked_ioctl = char_drv_ioctl,
};

static int __init char_drv_init(void) {
    int ret = 0;
    dev_t dev_num, dev;

    c_dev = cdev_alloc();

    ret = alloc_chrdev_region(&dev_num, min_minor, max_nr_devs, DEV_CHAR_REGION);
    if (ret < 0) {
        printk(KERN_ERR "device allocation failed");
        ret = -ENODEV;
        goto err_exit;
    }

    major = MAJOR(dev_num);
    dev = MKDEV(major,0);

    printk(KERN_INFO "datto char device major number %d", major);

    c_dev->owner = THIS_MODULE; 
    c_dev->ops = &char_fops;

    ret = cdev_add(c_dev, dev, 1);
    if (ret < 0) {
        printk(KERN_ERR "failed to add char device");
        goto clean_up;
    }

    char_dev_class = class_create(THIS_MODULE, DEV_CLASS);

    if (IS_ERR(char_dev_class)) {
        ret = -1;
        printk(KERN_ERR "fail to create char_dev_class");
        goto clean_up;
    }

    device_create(char_dev_class, NULL, dev, NULL, DEV_NAME);

    sema_init(&dev_sem, 1);
    init_rwsem(&rw_sem);

    printk(KERN_INFO "datto char_drv_init complete successfully");
    return 0;

clean_up:
    unregister_chrdev_region(dev, max_nr_devs);
    cdev_del(c_dev);
err_exit:
    printk(KERN_INFO "datto char_drv_init complete result %d", ret);
    return ret;
}

static void __exit char_drv_exit(void) {
    dev_t dev = MKDEV(major, 0);
    device_destroy(char_dev_class, dev);
    class_destroy(char_dev_class);
    cdev_del(c_dev);
    unregister_chrdev_region(major, max_nr_devs);
    printk(KERN_INFO "datto char_drv_exit");
}

module_init(char_drv_init);
module_exit(char_drv_exit);

MODULE_AUTHOR("Xin Zhou");
MODULE_LICENSE("GPL v2");
