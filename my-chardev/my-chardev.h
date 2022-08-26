#ifndef MY_CHARDEV_H
#define MY_CHARDEV_H

#include <linux/printk.h>
#include <linux/kernel.h>

#define my_chardev_info(fmt, ...) \
	pr_info("[my_chardev]: "fmt, ##__VA_ARGS__)

#define my_chardev_err(fmt, ...) \
	pr_err("[my_chardev]: "fmt, ##__VA_ARGS__)

#define my_chardev_debug(fmt, ...) \
	pr_debug("[my_chardev]: "fmt, ##__VA_ARGS__)



#endif /* MY_CHARDEV_H */
