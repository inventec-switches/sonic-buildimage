#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/i2c/pca954x.h>

struct inv_i2c_board_info {
	int ch;
	int size;
	struct i2c_board_info *board_info;
};

#define bus_id(id)  (id)
static struct pca954x_platform_mode pca9641_modes_2[] = {
	{.adap_id = bus_id (2),},
};

static struct pca954x_platform_mode pca9641_modes_3[] = {
	{.adap_id = bus_id (3),},
};

static struct pca954x_platform_mode pca9641_modes_4[] = {
	{.adap_id = bus_id (4),},
};

static struct pca954x_platform_mode pca9641_modes_5[] = {
	{.adap_id = bus_id (5),},
};

static struct pca954x_platform_mode pca9641_modes_6[] = {
	{.adap_id = bus_id (6),},
};

static struct pca954x_platform_mode mux_modes_0[] = {
	{.adap_id = bus_id (7),}, {.adap_id = bus_id (8),},
	{.adap_id = bus_id (9),}, {.adap_id = bus_id (10),},
	{.adap_id = bus_id (11),}, {.adap_id = bus_id (12),},
	{.adap_id = bus_id (13),}, {.adap_id = bus_id (14),},
};

static struct pca954x_platform_mode mux_modes_0_0[] = {
	{.adap_id = bus_id (100),}, {.adap_id = bus_id (101),},
	{.adap_id = bus_id (102),}, {.adap_id = bus_id (103),},
	{.adap_id = bus_id (104),}, {.adap_id = bus_id (105),},
	{.adap_id = bus_id (106),}, {.adap_id = bus_id (107),},
};

static struct pca954x_platform_mode mux_modes_0_1[] = {
	{.adap_id = bus_id (108),}, {.adap_id = bus_id (109),},
	{.adap_id = bus_id (110),}, {.adap_id = bus_id (111),},
	{.adap_id = bus_id (112),}, {.adap_id = bus_id (113),},
	{.adap_id = bus_id (114),}, {.adap_id = bus_id (115),},
};

static struct pca954x_platform_mode mux_modes_0_2[] = {
	{.adap_id = bus_id (116),}, {.adap_id = bus_id (117),},
	{.adap_id = bus_id (118),}, {.adap_id = bus_id (119),},
	{.adap_id = bus_id (120),}, {.adap_id = bus_id (121),},
	{.adap_id = bus_id (122),}, {.adap_id = bus_id (123),},
};

static struct pca954x_platform_mode mux_modes_0_3[] = {
	{.adap_id = bus_id (124),}, {.adap_id = bus_id (125),},
	{.adap_id = bus_id (126),}, {.adap_id = bus_id (127),},
	{.adap_id = bus_id (128),}, {.adap_id = bus_id (129),},
	{.adap_id = bus_id (130),}, {.adap_id = bus_id (131),},
};

static struct pca954x_platform_mode mux_modes_0_4[] = {
	{.adap_id = bus_id (132),}, {.adap_id = bus_id (133),},
	{.adap_id = bus_id (134),}, {.adap_id = bus_id (135),},
	{.adap_id = bus_id (136),}, {.adap_id = bus_id (137),},
	{.adap_id = bus_id (138),}, {.adap_id = bus_id (139),},
};

static struct pca954x_platform_mode mux_modes_0_5[] = {
	{.adap_id = bus_id (140),}, {.adap_id = bus_id (141),},
	{.adap_id = bus_id (142),}, {.adap_id = bus_id (143),},
	{.adap_id = bus_id (144),}, {.adap_id = bus_id (145),},
	{.adap_id = bus_id (146),}, {.adap_id = bus_id (147),},
};

static struct pca954x_platform_mode mux_modes_0_6[] = {
	{.adap_id = bus_id (148),}, {.adap_id = bus_id (149),},
	{.adap_id = bus_id (150),}, {.adap_id = bus_id (151),},
	{.adap_id = bus_id (152),}, {.adap_id = bus_id (153),},
	{.adap_id = bus_id (154),}, {.adap_id = bus_id (155),},
};

static struct pca954x_platform_mode mux_modes_0_7[] = {
	{.adap_id = bus_id (156),}, {.adap_id = bus_id (157),},
	{.adap_id = bus_id (158),}, {.adap_id = bus_id (159),},
	{.adap_id = bus_id (160),}, {.adap_id = bus_id (161),},
	{.adap_id = bus_id (162),}, {.adap_id = bus_id (163),},
};

static struct pca954x_platform_data pca9641_data_2 = {
	.modes = pca9641_modes_2,
	.num_modes = 1,
};

static struct pca954x_platform_data pca9641_data_3 = {
	.modes = pca9641_modes_3,
	.num_modes = 1,
};

static struct pca954x_platform_data pca9641_data_4 = {
	.modes = pca9641_modes_4,
	.num_modes = 1,
};

static struct pca954x_platform_data pca9641_data_5 = {
	.modes = pca9641_modes_5,
	.num_modes = 1,
};

static struct pca954x_platform_data pca9641_data_6 = {
	.modes = pca9641_modes_6,
	.num_modes = 1,
};

static struct pca954x_platform_data mux_data_0 = {
	.modes = mux_modes_0,
	.num_modes = 8,
};

static struct pca954x_platform_data mux_data_0_0 = {
	.modes = mux_modes_0_0,
	.num_modes = 8,
};

static struct pca954x_platform_data mux_data_0_1 = {
	.modes = mux_modes_0_1,
	.num_modes = 8,
};

static struct pca954x_platform_data mux_data_0_2 = {
	.modes = mux_modes_0_2,
	.num_modes = 8,
};

static struct pca954x_platform_data mux_data_0_3 = {
	.modes = mux_modes_0_3,
	.num_modes = 8,
};

static struct pca954x_platform_data mux_data_0_4 = {
	.modes = mux_modes_0_4,
	.num_modes = 8,
};

static struct pca954x_platform_data mux_data_0_5 = {
	.modes = mux_modes_0_5,
	.num_modes = 8,
};

static struct pca954x_platform_data mux_data_0_6 = {
	.modes = mux_modes_0_6,
	.num_modes = 8,
};

static struct pca954x_platform_data mux_data_0_7 = {
	.modes = mux_modes_0_7,
	.num_modes = 8,
};

static struct i2c_board_info i2c_device_info0[] __initdata = {
	{"pca9641", 0, 0x74, &pca9641_data_2, 0, 0},	//PCA9641	//i2c-2
	{"pca9641", 0, 0x76, &pca9641_data_3, 0, 0},	//PCA9641	//i2c-3
	{"pca9641", 0, 0x73, &pca9641_data_4, 0, 0},	//PCA9641	//i2c-4
	{"pca9641", 0, 0x75, &pca9641_data_5, 0, 0},	//PCA9641	//i2c-5
};

static struct i2c_board_info i2c_device_info1[] __initdata = {
	{"pca9641",  0, 0x75, &pca9641_data_6, 0, 0},	//PCA9641	//i2c-6
};

static struct i2c_board_info i2c_device_info2[] __initdata = {
	{"inv_cpld", 0, 0x55, 0, 0, 0},			//CPLD-eeprom
};

static struct i2c_board_info i2c_device_info3[] __initdata = {
	{"inv_psu", 0, 0x58, 0, 0, 0},			//psu
	{"inv_psu", 0, 0x59, 0, 0, 0},			//psu
	{"inv_ucd90160", 0, 0x34, 0, 0, 0},		//power monitor
	{"inv_ucd90160", 0, 0x6B, 0, 0, 0},		//power monitor(cpu board)
};

static struct i2c_board_info i2c_device_info4[] __initdata = {
	{"adm1275",   0, 0x10, 0, 0, 0},		//hot plug IC
	{"inv_tmp75", 0, 0x49, 0, 0, 0},		//u9
	{"inv_tmp75", 0, 0x4A, 0, 0, 0},		//u10
	{"inv_tmp75", 0, 0x4B, 0, 0, 0},		//u8
	{"inv_tmp75", 0, 0x48, 0, 0, 0},		//cpu board
	{"inv_tmp75", 0, 0x4F, 0, 0, 0},		//fan board
};

static struct i2c_board_info i2c_device_info5[] __initdata = {
	{"pca9548", 0, 0x70, &mux_data_0, 0, 0},
};

static struct i2c_board_info i2c_device_info6[] __initdata = {
};

static struct i2c_board_info i2c_device_info7[] __initdata = {
	{"pca9548", 0, 0x72, &mux_data_0_0, 0, 0},
};

static struct i2c_board_info i2c_device_info8[] __initdata = {
	{"pca9548", 0, 0x72, &mux_data_0_1, 0, 0},
};

static struct i2c_board_info i2c_device_info9[] __initdata = {
	{"pca9548", 0, 0x72, &mux_data_0_2, 0, 0},
};

static struct i2c_board_info i2c_device_info10[] __initdata = {
	{"pca9548", 0, 0x72, &mux_data_0_3, 0, 0},
};

static struct i2c_board_info i2c_device_info11[] __initdata = {
	{"pca9548", 0, 0x72, &mux_data_0_4, 0, 0},
};

static struct i2c_board_info i2c_device_info12[] __initdata = {
	{"pca9548", 0, 0x72, &mux_data_0_5, 0, 0},
};

static struct i2c_board_info i2c_device_info13[] __initdata = {
	{"pca9548", 0, 0x72, &mux_data_0_6, 0, 0},
};

static struct i2c_board_info i2c_device_info14[] __initdata = {
	{"pca9548", 0, 0x72, &mux_data_0_7, 0, 0},
};

static struct inv_i2c_board_info i2cdev_list[] = {
	{bus_id (0), ARRAY_SIZE (i2c_device_info0), i2c_device_info0},	//SMBUS0
	{bus_id (1), ARRAY_SIZE (i2c_device_info1), i2c_device_info1},	//SML1
	{bus_id (2), ARRAY_SIZE (i2c_device_info2), i2c_device_info2},
	{bus_id (3), ARRAY_SIZE (i2c_device_info3), i2c_device_info3},
	{bus_id (4), ARRAY_SIZE (i2c_device_info4), i2c_device_info4},
	{bus_id (5), ARRAY_SIZE (i2c_device_info5), i2c_device_info5},
	{bus_id (6), ARRAY_SIZE (i2c_device_info6), i2c_device_info6},
	{bus_id (7), ARRAY_SIZE (i2c_device_info7), i2c_device_info7},
	{bus_id (8), ARRAY_SIZE (i2c_device_info8), i2c_device_info8},
	{bus_id (9), ARRAY_SIZE (i2c_device_info9), i2c_device_info9},
	{bus_id (10), ARRAY_SIZE (i2c_device_info10), i2c_device_info10},
	{bus_id (11), ARRAY_SIZE (i2c_device_info11), i2c_device_info11},
	{bus_id (12), ARRAY_SIZE (i2c_device_info12), i2c_device_info12},
	{bus_id (13), ARRAY_SIZE (i2c_device_info13), i2c_device_info13},
	{bus_id (14), ARRAY_SIZE (i2c_device_info14), i2c_device_info14},
};

#define INV_PLATFORM_CLIENT_MAX_NUM 50	/*A big enough number for sum of i2cdev_list[i].size */
static int client_list_index = 0;
static struct i2c_client *client_list[INV_PLATFORM_CLIENT_MAX_NUM] = { 0 };

/////////////////////////////////////////////////////////////////////////////////////////
static struct platform_device *device_i2c_gpio0;
static struct i2c_gpio_platform_data i2c_gpio_platdata0 = {
	.scl_pin = 58,				//494,
	.sda_pin = 75,				//511,

	.udelay = 5,				//5:100kHz
	.sda_is_open_drain = 0,
	.scl_is_open_drain = 0,
	.scl_is_output_only = 0
};

static int __init inv_platform_init (void)
{
	struct i2c_adapter *adap = NULL;
	struct i2c_client *e = NULL;
	int ret = 0;
	int i, j, k;

	//printk("%s  \n", __func__);

	//use i2c-gpio
	//register i2c gpio
	//config gpio58,75 to gpio function 58=32+3*8+2 75=32*2+8*1+3 gpio69=32*2+8*0+5
	outl (inl (0x533) | (1 << 2), 0x533);	//i2c-gpio sdl (GPIO58)
	outl (inl (0x541) | (1 << 3), 0x541);	//i2c-gpio sda (GPIO75)
	outl (inl (0x540) | (1 << 5), 0x540);	//RST_I2C_MUX_N (GPIO69)
	outl (inl (0x500) | (1 << 7), 0x500);	//SYS_RDY_N (GPIO7)
	outl (inl (0x501) | (1 << 7), 0x501);	//BMC_HEART_BEAT (GPIO15)
	outl (inl (0x503) | (1 << 2) | (1 << 3), 0x503);	//PSOC_HEART_BEAT(26),CPLD_HEART_BEAT(27)

	device_i2c_gpio0 = platform_device_alloc ("i2c-gpio", 1);
	if (!device_i2c_gpio0)
	{
		printk (KERN_ERR "i2c-gpio: platform_device_alloc fail\n");
		return -ENOMEM;
	}
	device_i2c_gpio0->name = "i2c-gpio";
	device_i2c_gpio0->id = 1;
	device_i2c_gpio0->dev.platform_data = &i2c_gpio_platdata0;

	ret = platform_device_add (device_i2c_gpio0);
	if (ret)
	{
		printk (KERN_ERR "i2c-gpio: platform_device_add fail %d\n", ret);
	}
	msleep (10);

	for (i = 0; i < ARRAY_SIZE (i2cdev_list); i++)
	{
		adap = i2c_get_adapter (i2cdev_list[i].ch);
		if (adap == NULL)
		{
			printk ("platform get channel %d adapter fail\n", i);
			continue;
		}
		i2c_put_adapter (adap);
		for (j = 0; j < i2cdev_list[i].size; j++)
		{
			for (k = 0; k < 300; k++)
			{
				e = i2c_new_device (adap, &i2cdev_list[i].board_info[j]);
				if (e == NULL)
				{
					msleep (10);
				}
				else
				{
					client_list[client_list_index] = e;
					client_list_index++;
					break;
				}
			}
			if (k == 300)
			{
				printk ("[%d][%d] i2c device load fail\n", i, j);
			}
		}
	}
	printk ("inv_platform_init done\n");
	return ret;
}

static void __exit inv_platform_exit (void)
{
	int i;

	for (i = client_list_index - 1; i >= 0; i--)
	{
		i2c_unregister_device (client_list[i]);
	}
	device_i2c_gpio0->dev.platform_data = NULL;
	platform_device_unregister (device_i2c_gpio0);
	printk ("inv_platform_exit done\n");
}

module_init (inv_platform_init);
module_exit (inv_platform_exit);

MODULE_SOFTDEP("pre: inv_psoc");

MODULE_AUTHOR ("Inventec");
MODULE_DESCRIPTION ("Platform devices");
MODULE_LICENSE ("GPL");
