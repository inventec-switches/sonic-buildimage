/*
 * Sample kobject implementation
 * * Copyright (C) 2004-2007 Greg Kroah-Hartman <greg@kroah.com>
 * Copyright (C) 2007 Novell Inc.
 *
 * Released under the GPL version 2 only.
 *
 */
#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/list.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/interrupt.h>
/*work queue*/
#include <linux/jiffies.h>
#include <linux/workqueue.h>

#include "sff_io.h"
#include "io_config_maple.h"

#define DEBUG_MODE (0)
//#define TEST_CODE
#if (DEBUG_MODE == 1)
#define SFF_IO_DEBUG(fmt, args...) \
    printk (KERN_INFO "%s: " fmt "\r\n", __FUNCTION__,  ##args)
#else
    #define SFF_IO_DEBUG(fmt, args...)
#endif
#define SFF_IO_INFO(fmt, args...) \
    printk (KERN_INFO "[SFF_IO]%s: " fmt "\r\n", __FUNCTION__,  ##args)
#define SFF_IO_ERR(fmt, args...) \
    printk_ratelimited (KERN_ERR "{SFF_IO]%s: " fmt "\r\n",__FUNCTION__,  ##args)

static unsigned int gpio_ioexpander_int = CPLD_EXPANDER_INT_N;
static int io_exps_id = 0;
static int io_exps_check_fail_count = 0;
//static irq_handler_t  ioexpander_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs);
static struct i2c_client *Cpld_client = NULL;
static struct mutex cpld_lock;
static int inv_i2c_smbus_read_byte_data(struct i2c_client *client, u8 offset);

static int present_st[PORT_NUM];
static int rxlos_st[PORT_NUM];
static int txfault_st[PORT_NUM];

static int port_num_get(void)
{
    return PORT_NUM;
}

u8 pca9555_input_reg[2] =
{
    0x00,
    0x01
};

u8 pca9555_output_reg[2] =
{
    0x02,
    0x03
};
u8 pca9555_config_reg[2] =
{
    0x06,
    0x07
};
struct sff_io_client_node {
    struct i2c_client *client;
    int i2c_ch;
    struct list_head list;
    struct mutex lock;
};
static LIST_HEAD(sff_io_client_list);

static struct sff_io_client_node *sff_io_client_find(int i2c_ch, u8 slave_addr)
{
    struct list_head   *list_node = NULL;
    struct sff_io_client_node *node = NULL;
    int find = 0;
    list_for_each(list_node, &sff_io_client_list)
    {
        node = list_entry(list_node, struct sff_io_client_node, list);

        if (node->i2c_ch == i2c_ch) {
            find = 1;
            node->client->addr = slave_addr;
            break;
        }
    }

    if(!find)
    {
        SFF_IO_ERR("%s fail/n", __FUNCTION__);
        return NULL;
    }
    return node;
}

static int inv_i2c_smbus_read_word_data(struct i2c_client *client, u8 offset)
{
    int ret = 0;
    int i = 0;

    for (i = 0; i < RETRY_COUNT; i++) {

        ret = i2c_smbus_read_word_data(client, offset);
        if (ret < 0) {
            msleep(RETRY_DELAY_MS);
            continue;
        }
        break;
    }

    if (i >= RETRY_COUNT) {
        SFF_IO_ERR("%s fail:offset:0x%x try %d/%d! Error Code: %d\n", __func__, offset, i, RETRY_COUNT, ret);
    }

    return ret;
}

static int inv_i2c_smbus_write_word_data(struct i2c_client *client, u8 offset, u16 buf)
{
    int i;
    int ret = 0;
    for(i=0; i< RETRY_COUNT; i++)
    {
        ret = i2c_smbus_write_word_data(client, offset, buf);

        if (ret < 0) {
            msleep(RETRY_DELAY_MS);
            continue;
        }
        break;
    }
    if (i >= RETRY_COUNT) {
        SFF_IO_ERR("%s fail:offset:0x%x try %d/%d! Error Code: %d\n", __func__, offset, i, RETRY_COUNT, ret);
    }
    return ret;
}
static int inv_i2c_smbus_write_byte_data(struct i2c_client *client, u8 offset, u8 buf)
{
    int i;
    int ret = 0;
    for(i=0; i< RETRY_COUNT; i++)
    {
        ret = i2c_smbus_write_byte_data(client, offset, buf);

        if (ret < 0) {
            msleep(RETRY_DELAY_MS);
            continue;
        }
        break;
    }
    if (i >= RETRY_COUNT) {
        SFF_IO_ERR("%s fail:offset:0x%x try %d/%d! Error Code: %d\n", __func__, offset, i, RETRY_COUNT, ret);
    }
    return ret;
}
static int inv_i2c_smbus_read_byte_data(struct i2c_client *client, u8 offset)
{
    int ret = 0;
    int i = 0;

    for (i = 0; i < RETRY_COUNT; i++) {

        ret = i2c_smbus_read_byte_data(client, offset);
        if (ret < 0) {
            msleep(RETRY_DELAY_MS);
            continue;
        }
        break;
    }

    if (i >= RETRY_COUNT) {
        printk("%s fail:offset:0x%x try %d/%d! Error Code: %d\n", __func__, offset, i, RETRY_COUNT, ret);
    }

    return ret;
}
static int pca9555_word_read(u8 ch, u8 addr, u8 offset)
{

    struct sff_io_client_node *node = NULL;
    int ret = 0;
    node = sff_io_client_find(ch, addr);

    if(IS_ERR_OR_NULL(node))
      return -1;
    mutex_lock(&node->lock);
    ret = inv_i2c_smbus_read_word_data(node->client, offset);
    mutex_unlock(&node->lock);

    return ret;
}

static int pca9555_word_write(u8 ch, u8 addr, u8 offset, u16 val)
{

    struct sff_io_client_node *node = NULL;
    int ret = 0;
    node = sff_io_client_find(ch, addr);

    if(IS_ERR_OR_NULL(node))
      return -1;
    mutex_lock(&node->lock);
    ret = inv_i2c_smbus_write_word_data(node->client, offset, val);
    mutex_unlock(&node->lock);

    return ret;
}

static int sff_io_client_create(int ch)
{
    struct i2c_client *client = NULL;
    struct sff_io_client_node *node = NULL;
    struct i2c_adapter *adap;

    node = kzalloc(sizeof(struct sff_io_client_node), GFP_KERNEL);
    if(IS_ERR_OR_NULL(node))
    {
        goto exit_err;
    }
    client = kzalloc(sizeof(struct i2c_client), GFP_KERNEL);
    if(IS_ERR_OR_NULL(client))
    {
       goto exit_kfree_node;
    }

    /*get i2c adapter*/
     adap = i2c_get_adapter(ch);
     client->adapter = adap;

    if (!i2c_check_functionality(client->adapter,
        I2C_FUNC_SMBUS_BYTE_DATA | I2C_FUNC_SMBUS_WORD_DATA))
    {

        SFF_IO_ERR("%s: i2c check fail\n", __FUNCTION__);
        goto exit_kfree_sff_io_client;

    }

    node->client = client;
    node->i2c_ch = ch;
    mutex_init(&node->lock);
    list_add(&node->list, &sff_io_client_list);

    SFF_IO_DEBUG("i2c ch:%d\n", ch);
    return 0;

    exit_kfree_sff_io_client:
        kfree(client);
    exit_kfree_node:
        kfree(node);
    exit_err:
    return -1;

}
static int sff_io_client_create_all(void)
{
     int idx = 0;
     int table_size = ARRAY_SIZE(io_i2c_ch_table);

    for(idx = 0; idx < table_size; idx++) {
        if(sff_io_client_create(io_i2c_ch_table[idx]) < 0)
        {
            return -1;
        }
    }

    return 0;
}

static int sff_io_client_destroy(int i2c_ch)
{
    struct list_head   *list_node = NULL;
    struct sff_io_client_node *sff_io_client_node = NULL;
    int found = 0;
    list_for_each(list_node, &sff_io_client_list)
    {
        sff_io_client_node = list_entry(list_node, struct sff_io_client_node, list);
        if(i2c_ch == sff_io_client_node->i2c_ch)
        {
            found = 1;
            break;
        }
    }

    if(found)
    {
        list_del(list_node);
        i2c_put_adapter(sff_io_client_node->client->adapter);
        kfree(sff_io_client_node->client);
        kfree(sff_io_client_node);

        return 0;
    }
    else
   {
        SFF_IO_DEBUG("fail\n");
        return -1;
   }
}

static int sff_io_client_destroy_all(void)
{
    int idx = 0;
    int table_size = ARRAY_SIZE(io_i2c_ch_table);
    for(idx = 0; idx < table_size; idx++)
    {
        if(sff_io_client_destroy(io_i2c_ch_table[idx]) < 0)
        {
            return -1;
        }
    }

    return 0;
}


static int cpld_ioexpander_init(void)
{
    int ret = 0;
    struct i2c_client *client = NULL;
    struct i2c_adapter *adap;
    client = kzalloc(sizeof(struct i2c_client), GFP_KERNEL);

    if (IS_ERR_OR_NULL(client)) {
        goto exit_err;
    }

    SFF_IO_INFO("start\n");
    /*get i2c adapter*/
    adap = i2c_get_adapter(CPLD_I2C_CH);
    client->adapter = adap;
    client->addr = CPLD_I2C_ADDR;

    if (!i2c_check_functionality(client->adapter,
    I2C_FUNC_SMBUS_BYTE_DATA | I2C_FUNC_SMBUS_WORD_DATA))
    {

        SFF_IO_ERR("%s: i2c check fail\n", __FUNCTION__);
        goto exit_kfree_sff_client;

    }
    Cpld_client = client;

    mutex_init(&cpld_lock);
    SFF_IO_INFO("init ok\n");
    return 0;

exit_kfree_sff_client:
    kfree(client);

exit_err:
    return -1;


}
int cpld_ioexp_isr_enable(void)
{
    int ret = 0;
    /*enable cpld ioexpander isr mode*/
    mutex_lock(&cpld_lock);
    ret = inv_i2c_smbus_write_byte_data(Cpld_client, 0x37, 0x01);
    mutex_unlock(&cpld_lock);
    if (ret < 0)
    {
        return ret;
    }

    return 0;

}
int ioexp_isr_reg_get(int *reg)
{
    int ret = 0;
    int sfp = 0;
    int qsfp = 0;

    if(IS_ERR_OR_NULL(reg))
    {
        SFF_IO_ERR("NULL ptr\n");
        return -1;
    }
    if(IS_ERR_OR_NULL(Cpld_client))
    {
        SFF_IO_ERR("NULL ptr\n");
        return -1;
    }
    mutex_lock(&cpld_lock);
    ret = inv_i2c_smbus_read_word_data(Cpld_client, 0x35);
    mutex_unlock(&cpld_lock);
    if (ret < 0)
    {
        SFF_IO_ERR("2 read fail\n");
        return ret;
    }
    /*sfp 0x35 bit 0:7 0x36 bit 0:4
     * qsfp 0x36 bit 7, merge it into one data*/
    sfp = ret & 0xfff;
    qsfp = (ret & 0x8000) >> 3;

    /*bit = 0: interrupt occurs , reverse it for easy logic*/

    *reg = (~(sfp|qsfp)) & 0x1fff;
    return 0;
}


int present_st_get(int port)
{
    int new_port = port;
    return present_st[new_port];
}
int rxlos_get(int port)
{
    int new_port = port;
    return rxlos_st[new_port];
}
int txfault_get(int port)
{
    int new_port = port;
    return txfault_st[new_port];
}

bool sff_io_is_plugged(int port)
{
    int new_port = port;
    return ((0 == present_st[new_port]) ? true : false);
}

struct io_exp_input_t *find_ioexp(struct io_exp_input_t map[], int port_min) {

    int i = 0;
    for (i = 0; map[i].ch != END_OF_TABLE; i++) {

		if (port_min == map[i].port_min) {
			return &map[i];
		}
	}
	return NULL;
}

int io_exps_check(void)
{
    struct io_exp_config_t *config = NULL;
    int ret = 0;
    int val = 0;

    /*when interrupt gpio is low => input change is hapenning , skip this round check*/
    val = gpio_get_value(gpio_ioexpander_int);
    if (0 == val) {
        return 0;
    }
    if (END_OF_TABLE == io_exp_config[io_exps_id].ch) {
        if (0 == io_exps_check_fail_count) {
          //  SFF_IO_INFO("pass idx:%d", io_exps_id);
        }
        io_exps_id = 0;
        io_exps_check_fail_count = 0;
    }

    config = &io_exp_config[io_exps_id];
    io_exps_id++;

    ret = pca9555_word_read(config->ch, config->addr, pca9555_config_reg[0]);
    if (ret < 0) {
        SFF_IO_ERR("io config read fail: ch:0x%x addr:0x%x val:0x%x\n", config->ch, config->addr, config->val);
         io_exps_check_fail_count++;
         return ret;
    }

    if ((u16)ret == config->val) {

        //SFF_IO_INFO("io config read check ok: ch:0x%x addr:0x%x val:0x%x\n", config->ch, config->addr, config->val);
    } else {

        SFF_IO_ERR("io config read check fail: ch:0x%x addr:0x%x val:0x%x\n", config->ch, config->addr, config->val);
        io_exps_check_fail_count++;
        return -1;
    }

    return 0;
}
int io_exp_isr_handler(void)
{
    int idx = 0;
    int input_change;
    int val = 0;
    int ret = 0;
    int bit = 0;
    int bit_min = 0;
    int bit_max = 0;
    int port= 0;
    int new_port = 0;
    int size = sizeof(input_change_table)/sizeof(input_change_table[0]);
    struct io_exp_input_t *ioexp = NULL;
    val = gpio_get_value(gpio_ioexpander_int);
    //SFF_IO_INFO("1.gpio:%d level: %d\n", gpio_ioexpander_int, value);
    if (val == 0) {
        //SFF_IO_INFO("1.gpio:%d level: %d\n", gpio_ioexpander_int, value);
        if ((ret = ioexp_isr_reg_get(&input_change)) < 0 ) {
            SFF_IO_ERR("read fail\n");
            return ret;
        }

        SFF_IO_INFO("input_change:0x%x\n", input_change);
        for (idx = 0; idx < size; idx++) {

            if (input_change & (1 << idx)) {
                /*present scan*/

                SFF_IO_DEBUG("input_change_table[%d]:%d\n", idx, input_change_table[idx]);
                ioexp = find_ioexp(io_exp_present, input_change_table[idx]);
                if (NULL == ioexp) {

                    SFF_IO_ERR("cant find io exp present: %d\n", input_change_table[idx]);
                    break;
                }
                ret = pca9555_word_read(ioexp->ch, ioexp->addr, pca9555_input_reg[0]);
                if (ret < 0) {
                    SFF_IO_ERR("present read word fail: ch:0x%x addr:0x%x\n", ioexp->ch, ioexp->addr);
                    return ret;
                }
                port = ioexp->port_min;
                bit_min = ioexp->bit_min;
                bit_max = ioexp->bit_max;

                for (bit = bit_min; bit <= bit_max; bit++) {
                    val = ((ret & (1 << bit)) ? 1 : 0);
                    new_port = ioexp_port_2_port[port];

                    SFF_IO_DEBUG("new_port:%d", new_port);
                    if (val != present_st[new_port]) {

                        if (!val) {
                            SFF_IO_INFO("port:%d plug in", new_port+1);
                        } else {
                            SFF_IO_INFO("port:%d plug out\n", new_port+1);
                        }
                    }
                    present_st[new_port] = val;

                    port++;
                }
                /*rx_los scan*/
                if (size-1 == idx) {

                    SFF_IO_DEBUG("qsfp doesnt have rxlos%d\n", idx);
                    break;
                }
                SFF_IO_DEBUG("input_change_table[%d]:%d\n", idx, input_change_table[idx]);
                ioexp = find_ioexp(io_exp_rxlos, input_change_table[idx]);
                if (NULL == ioexp) {

                    SFF_IO_ERR("cant find io exp rxlos: %d\n", input_change_table[idx]);
                    break;
                }
                port = ioexp->port_min;
                bit_min = ioexp->bit_min;
                bit_max = ioexp->bit_max;

                for (bit = bit_min; bit <= bit_max; bit++) {
                    val = ((ret & (1 << bit)) ? 1 : 0);
                    new_port = ioexp_port_2_port[port];
                    if (val != rxlos_st[new_port]) {

                        if (val) {
                            //SFF_IO_INFO("port:%d rxlos assert", new_port+1);
                        } else {
                            //SFF_IO_INFO("port:%d rxlos deassert\n", new_port+1);
                        }
                    }
                    rxlos_st[new_port] = val;
                    port++;
                }

                SFF_IO_DEBUG("input_change_table[%d]:%d\n", idx, input_change_table[idx]);
                ioexp = find_ioexp(io_exp_txfault, input_change_table[idx]);
                if (NULL == ioexp) {

                    SFF_IO_ERR("cant find io exp tx fault: %d\n", input_change_table[idx]);
                    break;
                }
                port = ioexp->port_min;
                bit_min = ioexp->bit_min;
                bit_max = ioexp->bit_max;

                for (bit = bit_min; bit <= bit_max; bit++) {
                    val = ((ret & (1 << bit)) ? 1 : 0);
                    new_port = ioexp_port_2_port[port];
                    if (val != txfault_st[new_port]) {

                        if (val) {
                            //SFF_IO_INFO("port:%d txfault assert", new_port+1);
                        } else {
                            //SFF_IO_INFO("port:%d txfault deassert\n", new_port+1);
                        }
                    }
                    txfault_st[new_port] = val;
                    port++;
                }

            }
        }
    }

    return ret;
}

int ioexpander_int_init(void)
{
    int result;
    int value = 0x0;
    result = gpio_is_valid(gpio_ioexpander_int);

    if (result < 0) {

        SFF_IO_ERR("valid gpio:%d ret:%d\n", gpio_ioexpander_int, result);
        return -1;
    }

    value = gpio_direction_input(gpio_ioexpander_int);
    if (value < 0 ) {

        SFF_IO_ERR("gpio:%d dir set. err code:%d\n", gpio_ioexpander_int, value);
        return -1;

    }
    value = gpio_get_value(gpio_ioexpander_int);
    SFF_IO_INFO("ok gpio:%d value:%d\n", gpio_ioexpander_int, value);

    if (cpld_ioexpander_init() < 0) {
        gpio_free(gpio_ioexpander_int);
        return -1;
    }
    return 0;
}

int ioexpander_int_deinit(void)
{
    if (Cpld_client) {

        i2c_put_adapter(Cpld_client->adapter);
        kfree(Cpld_client);
    }
    gpio_free(gpio_ioexpander_int);
    return 0;
}
static int io_exps_config(void)
{
    int count = 0;
    int ret = 0;
    struct io_exp_config_t *config = NULL;
    /*config io exp*/
    for (count = 0; io_exp_config[count].ch != END_OF_TABLE; count++) {
        config = &io_exp_config[count];
        ret = pca9555_word_write(config->ch, config->addr, pca9555_config_reg[0], config->val);
        if (ret < 0) {
            SFF_IO_ERR("io config fail: ch:0x%x addr:0x%x val:0x%x\n", config->ch, config->addr, config->val);
            return ret;
        }
    }
    return 0;
}

int sff_io_init(void)
{

	int val = 0;
    int count = 0;
    int port = 0;
    int new_port = 0;
    int bit = 0;
    int bit_min = 0;
    int bit_max = 0;
    int ret = 0;
    struct io_exp_input_t *ioexp = NULL;
    //struct io_exp_config_t *config = NULL;
    SFF_IO_DEBUG("init start\n");
    if(sff_io_client_create_all() < 0)
      return -1;
    SFF_IO_DEBUG("sff_io_client_create_all ok\n");
    /*
    if(sff_io_mux_init() < 0)
      return -1;
    */
    /*config io exp*/
    ret = io_exps_config();
    if (ret < 0) {
        return ret;
    }
    /*fist present scan*/
    for (count = 0; io_exp_present[count].ch != END_OF_TABLE; count++) {
        ioexp = &io_exp_present[count];
        ret = pca9555_word_read(ioexp->ch, ioexp->addr, pca9555_input_reg[0]);
        if (ret < 0) {
            SFF_IO_ERR("read word fail: %d\n", ioexp->ch);
            return ret;
        }
        port = ioexp->port_min;
        bit_min = ioexp->bit_min;
        bit_max = ioexp->bit_max;
        for (bit = bit_min; bit <= bit_max; bit++) {
            val = ((ret & (1 << bit)) ? 1 : 0);
            new_port = ioexp_port_2_port[port];

            SFF_IO_DEBUG("new_port:%d", new_port);
            present_st[new_port] = val;

            port++;
        }
    }
    /*fist rxlos scan*/
    for (count = 0; io_exp_rxlos[count].ch != END_OF_TABLE; count++) {
        ioexp = &io_exp_rxlos[count];
        ret = pca9555_word_read(ioexp->ch, ioexp->addr, pca9555_input_reg[0]);
        if (ret < 0) {
            SFF_IO_ERR("read word fail: %d\n", ioexp->ch);
            return ret;
        }
        port = ioexp->port_min;
        bit_min = ioexp->bit_min;
        bit_max = ioexp->bit_max;
        for (bit = bit_min; bit <= bit_max; bit++) {
            val = ((ret & (1 << bit)) ? 1 : 0);
            new_port = ioexp_port_2_port[port];

            SFF_IO_DEBUG("new_port:%d", new_port);
            rxlos_st[new_port] = val;

            port++;
        }
    }
    /*fist txfault scan*/
    for (count = 0; io_exp_txfault[count].ch != END_OF_TABLE; count++) {
        ioexp = &io_exp_txfault[count];
        ret = pca9555_word_read(ioexp->ch, ioexp->addr, pca9555_input_reg[0]);
        if (ret < 0) {
            SFF_IO_ERR("read word fail: %d\n", ioexp->ch);
            return ret;
        }
        port = ioexp->port_min;
        bit_min = ioexp->bit_min;
        bit_max = ioexp->bit_max;
        for (bit = bit_min; bit <= bit_max; bit++) {
            val = ((ret & (1 << bit)) ? 1 : 0);
            new_port = ioexp_port_2_port[port];

            SFF_IO_DEBUG("new_port:%d", new_port);
            txfault_st[new_port] = val;

            port++;
        }
    }

    if (ioexpander_int_init() < 0) {
        return -1;
    }
    SFF_IO_DEBUG("OK\n");
    return 0;

}
void sff_io_deinit(void)
{
     ioexpander_int_deinit();
     sff_io_client_destroy_all();
}
#if 0
static int __init example_init(void)
{
    return sff_io_init();
}

static void __exit example_exit(void)
{
    sff_io_deinit();
}

module_init(example_init);
module_exit(example_exit);
#endif
//MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Alang");
