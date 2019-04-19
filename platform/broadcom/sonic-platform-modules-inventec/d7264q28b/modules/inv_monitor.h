#ifndef _INV_MONITOR_H_
#define _INV_MONITOR_H_

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sysfs.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/workqueue.h>
#include <linux/jiffies.h>
#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/fs.h>


/* Module information */
#define MONITOR_AUTHOR            "James <Huang.James@inventec.com>, Neil <liao.neil@inventec.com>"
#define MONITOR_DESC              "Inventec HW monitoring SYSFS for JD"
#define MONITOR_VERSION           "1.0"
#define MONITOR_LICENSE           "GPL"


#define CORE_CLS_NAME          "core"
#define FAN_CLS_NAME           "fan"
#define PSU_CLS_NAME           "psus"


#define MONITOR_INFO(fmt, args...) printk( KERN_INFO "[MONITOR] " fmt, ##args)
#define MONITOR_WARN(fmt, args...) printk( KERN_WARNING "[MONITOR] " fmt, ##args)
#define MONITOR_ERR(fmt, args...)  printk( KERN_ERR  "[MONITOR] " fmt, ##args)

#endif /* _INV_MONITOR_H_ */

