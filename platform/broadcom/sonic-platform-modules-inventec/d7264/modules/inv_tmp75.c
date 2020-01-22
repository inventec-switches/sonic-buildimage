/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>

struct tmp75_data {
	struct device *hwmon_dev;
	u8 last_value;
};

static ssize_t read_temperature(struct device *dev, struct device_attribute *da, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	s32 temp = i2c_smbus_read_word_data(client, 0);
	if(temp<0) 
	{
		return  sprintf(buf, "0\n");
	}
	return sprintf(buf, "%d\n",  ((temp & 0xff) * 1000 + ((temp>>14 & 0x03)*250)*((temp & 0x80)?-1:1)));
}

static SENSOR_DEVICE_ATTR(temp1_input, S_IRUGO, read_temperature, 0, 0);

static struct attribute *tmp75_attributes[] = {
	&sensor_dev_attr_temp1_input.dev_attr.attr,
	NULL
};

static const struct attribute_group tmp75_group = {
	.attrs = tmp75_attributes,
};

/*-----------------------------------------------------------------------*/
/* device probe and removal */
static int
inv_tmp75_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct tmp75_data *data;
	int status;
   
	if (!i2c_check_functionality(client->adapter,
			I2C_FUNC_SMBUS_BYTE_DATA | I2C_FUNC_SMBUS_WORD_DATA))
		return -EIO;

	data = kzalloc(sizeof(struct tmp75_data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	i2c_set_clientdata(client, data);

	/* Register sysfs hooks */
	status = sysfs_create_group(&client->dev.kobj, &tmp75_group);

	if (status)
		goto exit_free;

	data->hwmon_dev = hwmon_device_register(&client->dev);
	if (IS_ERR(data->hwmon_dev)) {
		status = PTR_ERR(data->hwmon_dev);
		goto exit_remove;
	}
	
	return 0;

exit_remove:
	sysfs_remove_group(&client->dev.kobj, &tmp75_group);
exit_free:
	i2c_set_clientdata(client, NULL);
	kfree(data);
	return status;
}

static int inv_tmp75_remove(struct i2c_client *client)
{	
	struct tmp75_data *data = i2c_get_clientdata(client);

	sysfs_remove_group(&client->dev.kobj, &tmp75_group);
	hwmon_device_unregister(data->hwmon_dev);
	i2c_set_clientdata(client, NULL);	
	kfree(data);
	return 0;
}

static const struct i2c_device_id inv_tmp75_ids[] = {
	{ "inv_tmp75" , 0, },
	{ /* LIST END */ }
};
MODULE_DEVICE_TABLE(i2c, inv_tmp75_ids);

static struct i2c_driver tmp75_driver = {
	.class		= I2C_CLASS_HWMON,
	.driver = {
		.name	= "inv_tmp75",
	},
	.probe		= inv_tmp75_probe,
	.remove		= inv_tmp75_remove,
	.id_table	= inv_tmp75_ids,
};

static int __init inv_tmp75_init(void)
{
	return i2c_add_driver(&tmp75_driver);
}

static void __exit inv_tmp75_exit(void)
{
	i2c_del_driver(&tmp75_driver);
}

MODULE_AUTHOR("jack.ting <ting.jack@inventec>");
MODULE_DESCRIPTION("inventec tmp75 driver");
MODULE_LICENSE("GPL");

module_init(inv_tmp75_init);
module_exit(inv_tmp75_exit);
