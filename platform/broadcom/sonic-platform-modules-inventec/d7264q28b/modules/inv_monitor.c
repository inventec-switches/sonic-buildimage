#include "inv_monitor.h"
#include "inv_proxy.h"

static struct class *core_class_p = NULL;
static struct class *psu_class_p = NULL;
static struct class *fan_class_p = NULL;

static int psu_major;

#define NUM_PSU      (2)
#define NUM_PSU_TEMP (2)
#define NUM_FAN      (4)

/* CORE */
#define HWMON_DEV_PATH_CORE0_TEMP	     "/sys/class/hwmon/hwmon0/temp2_input"
#define HWMON_DEV_PATH_CORE1_TEMP	     "/sys/class/hwmon/hwmon0/temp3_input"
#define HWMON_DEV_PATH_CORE2_TEMP	     "/sys/class/hwmon/hwmon0/temp4_input"
#define HWMON_DEV_PATH_CORE3_TEMP	     "/sys/class/hwmon/hwmon0/temp5_input"

/* PSU */
#define HWMON_DEV_PATH_PSU0_PRESENT	     "/sys/class/hwmon/hwmon1/psu0"
#define HWMON_DEV_PATH_PSU1_PRESENT	     "/sys/class/hwmon/hwmon1/psu1"
#define HWMON_DEV_PATH_PSU0_VOLTAGE_IN   "/sys/class/hwmon/hwmon1/psoc_psu1_vin"
#define HWMON_DEV_PATH_PSU1_VOLTAGE_IN   "/sys/class/hwmon/hwmon1/psoc_psu2_vin"
#define HWMON_DEV_PATH_PSU0_VOLTAGE_OUT  "/sys/class/hwmon/hwmon1/psoc_psu1_vout"
#define HWMON_DEV_PATH_PSU1_VOLTAGE_OUT  "/sys/class/hwmon/hwmon1/psoc_psu2_vout"
#define HWMON_DEV_PATH_PSU0_POWER_IN     "/sys/class/hwmon/hwmon1/psoc_psu1_pin"
#define HWMON_DEV_PATH_PSU1_POWER_IN     "/sys/class/hwmon/hwmon1/psoc_psu2_pin"
#define HWMON_DEV_PATH_PSU0_POWER_OUT    "/sys/class/hwmon/hwmon1/psoc_psu1_pout"
#define HWMON_DEV_PATH_PSU1_POWER_OUT    "/sys/class/hwmon/hwmon1/psoc_psu2_pout"
#define HWMON_DEV_PATH_PSU0_TEMP1        "/sys/class/hwmon/hwmon1/thermal_psu1"
#define HWMON_DEV_PATH_PSU1_TEMP1        "/sys/class/hwmon/hwmon1/thermal_psu2"
#define HWMON_DEV_PATH_PSU0_TEMP2        "/sys/class/hwmon/hwmon1/thermal2_psu1"
#define HWMON_DEV_PATH_PSU1_TEMP2        "/sys/class/hwmon/hwmon1/thermal2_psu2"

/* FAN */
#define HWMON_DEV_PATH_FAN1_FRONT_RPM    "/sys/class/hwmon/hwmon1/fan1_input"
#define HWMON_DEV_PATH_FAN1_REAR_RPM     "/sys/class/hwmon/hwmon1/fan2_input"
#define HWMON_DEV_PATH_FAN2_FRONT_RPM    "/sys/class/hwmon/hwmon1/fan3_input"
#define HWMON_DEV_PATH_FAN2_REAR_RPM     "/sys/class/hwmon/hwmon1/fan4_input"
#define HWMON_DEV_PATH_FAN3_FRONT_RPM    "/sys/class/hwmon/hwmon1/fan5_input"
#define HWMON_DEV_PATH_FAN3_REAR_RPM     "/sys/class/hwmon/hwmon1/fan6_input"
#define HWMON_DEV_PATH_FAN4_FRONT_RPM    "/sys/class/hwmon/hwmon1/fan7_input"
#define HWMON_DEV_PATH_FAN4_REAR_RPM     "/sys/class/hwmon/hwmon1/fan8_input"
#define HWMON_DEV_PATH_FAN_PRESENT       "/sys/class/hwmon/hwmon1/fan_gpi"

struct note
{
    int index;
    char *name;
    struct dev *dev;
    char *present;
    char *in_voltage;
    char *out_voltage;
    char *in_power;
    char *out_power;
    char *temp1;
    char *temp2;
    char *temp3;
};

struct note psu_note[] = {
    { 0, "psu1", NULL, HWMON_DEV_PATH_PSU0_PRESENT, \
                       HWMON_DEV_PATH_PSU0_VOLTAGE_IN, \
                       HWMON_DEV_PATH_PSU0_VOLTAGE_OUT, \
                       HWMON_DEV_PATH_PSU0_POWER_IN, \
                       HWMON_DEV_PATH_PSU0_POWER_OUT, \
                       HWMON_DEV_PATH_PSU0_TEMP1, \
                       HWMON_DEV_PATH_PSU0_TEMP2 },
    { 1, "psu2", NULL, HWMON_DEV_PATH_PSU1_PRESENT, \
                       HWMON_DEV_PATH_PSU1_VOLTAGE_IN, \
                       HWMON_DEV_PATH_PSU1_VOLTAGE_OUT, \
                       HWMON_DEV_PATH_PSU1_POWER_IN, \
                       HWMON_DEV_PATH_PSU1_POWER_OUT, \
                       HWMON_DEV_PATH_PSU1_TEMP1, \
                       HWMON_DEV_PATH_PSU1_TEMP2 },
};


static ssize_t show_attr_psu_temperature(struct device *dev,
                                     struct device_attribute *attr,
                                     char *buf)
{
    int integer = 0;
    int floater = 0;
    int modulus = 0;
    int present = DEBUG_INV_INT;
    int result = DEBUG_INV_INT;
    char name[MAX_PATH_SIZE];

    memset(name, 0, MAX_PATH_SIZE);
	snprintf(name, MAX_PATH_SIZE, "%s", attr->attr.name);

    struct note *notep = NULL ;
    notep = dev_get_drvdata(dev);

    if (!notep)
        goto err_get_psu_module_temperature;

    present = read_psu_present(notep->present) ;
    if ( present == 0 )
        goto err_get_psu_module_temperature;

    if ( strncmp(name, "temp1", strlen(name)) == 0 )
        result = read_psu_temperature(notep->temp1) ;
    else if ( strncmp(name, "temp2", strlen(name)) == 0 )
        result = read_psu_temperature(notep->temp2) ;
    else if ( strncmp(name, "temp3", strlen(name)) == 0 )
        result = read_psu_temperature(notep->temp3) ;
    else
        goto err_get_psu_module_temperature;

    if ( result == -1 )
        goto err_get_psu_module_temperature;

    integer = result / 1000 ;
    if ( result > 0 )
    {
        modulus = result % 1000 ;
        floater = modulus / 100 ;
        return snprintf(buf, MAX_PATH_SIZE, "+%d.%d C", integer, floater );
    }
    else
    {
        modulus = -result % 1000 ;
        floater = modulus / 100 ;
        return snprintf(buf, MAX_PATH_SIZE, "%d.0 C", integer, floater );
    }

err_get_psu_module_temperature :
    return sprintf(buf, "N/A");
}

static ssize_t show_attr_psu_voltage(struct device *dev,
                                     struct device_attribute *attr,
                                     char *buf)
{
    int integer = 0;
    int floater = 0;
    int modulus = 0;
    int present = DEBUG_INV_INT;
    int result = DEBUG_INV_INT;
    char name[MAX_PATH_SIZE];

    memset(name, 0, MAX_PATH_SIZE);
	snprintf(name, MAX_PATH_SIZE, "%s", attr->attr.name);

    struct note *notep = NULL ;
    notep = dev_get_drvdata(dev);

    if (!notep)
        goto err_get_psu_module_voltage;

    present = read_psu_present(notep->present) ;
    if ( present == 0 )
        goto err_get_psu_module_voltage;

    if ( strncmp(name, "in_voltage", strlen(name)) == 0 )
        result = read_psu_voltage(notep->in_voltage) ;
    else if ( strncmp(name, "out_voltage", strlen(name)) == 0 )
        result = read_psu_voltage(notep->out_voltage) ;
    else
        goto err_get_psu_module_voltage;

    if ( result == -1 )
        goto err_get_psu_module_voltage;

    integer = result / 1000 ;
    if ( result > 0 )
    {
        modulus = result % 1000 ;
        floater = modulus / 10 ;
        return snprintf(buf, MAX_PATH_SIZE, "+%d.%02d V", integer, floater);
    }
    else
    {
        modulus = -result % 1000 ;
        floater = modulus / 10 ;
        return snprintf(buf, MAX_PATH_SIZE, "%d.%02d V", integer, floater);
    }

err_get_psu_module_voltage :
    return sprintf(buf, "N/A");
}

static ssize_t show_attr_psu_power(struct device *dev,
                                     struct device_attribute *attr,
                                     char *buf)
{
    int integer = 0;
    int floater = 0;
    int modulus = 0;
    int present = DEBUG_INV_INT;
    int result = DEBUG_INV_INT;
    char name[MAX_PATH_SIZE];

    memset(name, 0, MAX_PATH_SIZE);
	snprintf(name, MAX_PATH_SIZE, "%s", attr->attr.name);

    struct note *notep = NULL ;
    notep = dev_get_drvdata(dev);

    if (!notep)
        goto err_get_psu_module_power;

    present = read_psu_present(notep->present) ;
    if ( present == 0 )
        goto err_get_psu_module_power;

    if ( strncmp(name, "in_power", strlen(name)) == 0 )
        result = read_psu_power(notep->in_power) ;
    else if ( strncmp(name, "out_power", strlen(name)) == 0 )
        result = read_psu_power(notep->out_power) ;
    else
        goto err_get_psu_module_power;

    if ( result == -1 )
        goto err_get_psu_module_power;

    integer = result / 1000 ;
    if ( result > 0 )
    {
        modulus = result % 1000 ;
        floater = modulus / 10 ;
        return snprintf(buf, MAX_PATH_SIZE, "+%d.%02d W", integer, floater);
    }
    else
    {
        modulus = -result % 1000 ;
        floater = modulus / 10 ;
        return snprintf(buf, MAX_PATH_SIZE, "%d.%02d W", integer, floater);
    }
    goto err_get_psu_module_power;

err_get_psu_module_power :
    return sprintf(buf, "N/A");
}

static ssize_t show_attr_psu_present(struct device *dev,
                                     struct device_attribute *attr,
                                     char *buf)
{
    int result = DEBUG_INV_INT;

    struct note *notep = NULL ;
    notep = dev_get_drvdata(dev);

    if (!notep)
        goto err_get_psu_module_present;

    result = read_psu_present(notep->present) ;
    if ( result > -1 )
        return snprintf(buf, MAX_PATH_SIZE, "%d", result);
    else
        goto err_get_psu_module_present;

    goto err_get_psu_module_present;

err_get_psu_module_present :
    return sprintf(buf, "N/A");
}

static ssize_t show_attr_psu_temperature_number(struct device *dev,
                                     struct device_attribute *attr,
                                     char *buf)
{
	return sprintf(buf, "%d", NUM_PSU_TEMP);
}

static ssize_t show_attr_psu_number(struct class *class,
                                     struct class_attribute *attr,
                                     char *buf)
{
	return sprintf(buf, "%d", NUM_PSU);
}

static DEVICE_ATTR(present,            S_IRUGO,         show_attr_psu_present,                   NULL);
static DEVICE_ATTR(temp_nm,            S_IRUGO,         show_attr_psu_temperature_number,        NULL);
static DEVICE_ATTR(in_voltage,         S_IRUGO,         show_attr_psu_voltage,                   NULL);
static DEVICE_ATTR(out_voltage,        S_IRUGO,         show_attr_psu_voltage,                   NULL);
static DEVICE_ATTR(in_power,           S_IRUGO,         show_attr_psu_power,                     NULL);
static DEVICE_ATTR(out_power,          S_IRUGO,         show_attr_psu_power,                     NULL);
static DEVICE_ATTR(temp1,              S_IRUGO,         show_attr_psu_temperature,               NULL);
static DEVICE_ATTR(temp2,              S_IRUGO,         show_attr_psu_temperature,               NULL);

static int psu_device_match(struct device *dev, const void *data)
{
    return !strcmp(dev_name(dev), (const char *)data);
}

static int register_psu_device(void)
{
    dev_t psu_devt  = 0;
    int index = 0 ;
    struct device *global_device_p = NULL;
    dev_t global_dev_num = 0 ;

    /* Register device number */
    if (alloc_chrdev_region(&psu_devt, 0, NUM_PSU, "psus") < 0){
        MONITOR_ERR("Allocate PSU MAJOR failure! \n");
        goto err_register_psu_module_1;
    }
    psu_major  = MAJOR(psu_devt);

    for ( index ; index < NUM_PSU ; index++ )
    {
        // struct note *note=NULL;
        struct device *device_p = NULL;

        dev_t dev_num  = MKDEV(psu_major, 0);

        dev_num = MKDEV(psu_major, index) ;

        device_p = device_create(psu_class_p,   /* struct class *cls     */
                             NULL,          /* struct device *parent */
                             dev_num,       /* dev_t devt            */
                             NULL,   /* void *private_data    */
                             psu_note[index].name);     /* const char *fmt       */

        psu_note[index].dev = device_p;

        dev_set_drvdata(device_p, &psu_note[index]);

        MONITOR_INFO("device_create named '%s' \n", psu_note[index].name);

        if (IS_ERR(device_p)){
            MONITOR_ERR("Allocate PSU device_create failure! \n");
            goto err_register_psu_module_2;
        }
        if (device_create_file(device_p, &dev_attr_present) < 0 ){
            MONITOR_ERR("Allocate PSU device attribute named 'temp3' failed \n");
            goto err_register_psu_module_3;
        }
        if (device_create_file(device_p, &dev_attr_temp_nm) < 0 ){
            MONITOR_ERR("Allocate PSU device attribute named 'temp_nm' failed \n");
            goto err_register_psu_module_3;
        }
        if (device_create_file(device_p, &dev_attr_in_voltage) < 0 ){
            MONITOR_ERR("Allocate PSU device attribute named 'in_voltage' failed \n");
            goto err_register_psu_module_3;
        }
        if (device_create_file(device_p, &dev_attr_out_voltage) < 0 ){
            MONITOR_ERR("Allocate PSU device attribute named 'out_voltage' failed \n");
            goto err_register_psu_module_3;
        }
        if (device_create_file(device_p, &dev_attr_in_power) < 0 ){
            MONITOR_ERR("Allocate PSU device attribute named 'in_power' failed \n");
            goto err_register_psu_module_3;
        }
        if (device_create_file(device_p, &dev_attr_out_power) < 0 ){
            MONITOR_ERR("Allocate PSU device attribute named 'out_power' failed \n");
            goto err_register_psu_module_3;
        }
        if (device_create_file(device_p, &dev_attr_temp1) < 0 ){
            MONITOR_ERR("Allocate PSU device attribute named 'temp1' failed \n");
            goto err_register_psu_module_3;
        }
        if (device_create_file(device_p, &dev_attr_temp2) < 0 ){
            MONITOR_ERR("Allocate PSU device attribute named 'temp2' failed \n");
            goto err_register_psu_module_3;
        }
    }
    return 0;
err_register_psu_module_1:
    unregister_chrdev_region(MKDEV(psu_major, 0), NUM_PSU);
err_register_psu_module_2:
    return -1;
err_register_psu_module_3:
    device_unregister(global_device_p);
    device_destroy(psu_class_p, global_dev_num);
}

int
remove_psu_device(void)
{
    dev_t dev_num;
    char dev_name[MAX_PATH_SIZE];
    struct device *device_p;
    int minor_curr;

    for ( minor_curr = 0 ; minor_curr < NUM_PSU; minor_curr++ )
    {
        memset(dev_name, 0, sizeof(dev_name));
        snprintf(dev_name, sizeof(dev_name), "%s%d", "psu", minor_curr+1);
        device_p = class_find_device(psu_class_p, NULL, dev_name, psu_device_match);
        if (!device_p){
            continue;
        }
        dev_num = MKDEV(psu_major, minor_curr);
        MONITOR_INFO("Found dev Num is %d \n", dev_num);
        // device_unregister(device_p);
        device_destroy(psu_class_p, dev_num);
    }
    MONITOR_INFO("Inventec monitor remove_psu_device success.\n", MONITOR_VERSION);
}


static struct class_attribute psu_class_attrs[] = {
	__ATTR(psu_nm, S_IRUGO, show_attr_psu_number, NULL),
	__ATTR_NULL,
};

int
parsing_fan_present( char *attr, int index )
{
    int result = DEBUG_INV_INT;
    int binary = 0 ;

    result = read_fan_present(attr) ;

    // if the specific FAN is not present, the psoc driver will raise up the specific bit to 1 .
    // 0001 indicates that the first fan has been removed.
    // 0010 indicates that the second fan has been removed.
    // 0100 indicates that the third fan has been removed.
    // 1000 indicates that the fourth fan has been removed.
    // Therefore, we need to reverse the results to match customer needs.
    binary = result >> index;
    MONITOR_INFO("parsing_fan_present binary(%d)=result(%d)>>index(%d).\n", binary, result, index);
    if (binary & 1)
        result=0 ;
    else
        result=1 ;
    return result;
}

static ssize_t show_attr_fan_present(struct class *class,
                                     struct class_attribute *attr,
                                     char *buf)
{
    char name[MAX_PATH_SIZE];
    int result = DEBUG_INV_INT;

    memset(name, 0, MAX_PATH_SIZE);

	snprintf(name, MAX_PATH_SIZE, "%s", attr->attr.name);

    if ( strncmp(name, "fan1_present", strlen(name)) == 0 )
        result = parsing_fan_present( HWMON_DEV_PATH_FAN_PRESENT, 0 );
    else if ( strncmp(name, "fan2_present", strlen(name)) == 0 )
        result = parsing_fan_present( HWMON_DEV_PATH_FAN_PRESENT, 1 );
    else if ( strncmp(name, "fan3_present", strlen(name)) == 0 )
        result = parsing_fan_present( HWMON_DEV_PATH_FAN_PRESENT, 2 );
    else if ( strncmp(name, "fan4_present", strlen(name)) == 0 )
        result = parsing_fan_present( HWMON_DEV_PATH_FAN_PRESENT, 3 );
    else
        goto err_get_fan_module_present ;

    if ( result > -1 )
        return snprintf(buf, MAX_PATH_SIZE, "%d", result);
    else
        goto err_get_fan_module_present;

err_get_fan_module_present :
    return sprintf(buf, "N/A");
}

static ssize_t show_attr_fan_number(struct class *class,
                                     struct class_attribute *attr,
                                     char *buf)
{
	return sprintf(buf, "%d", NUM_FAN);
}

static ssize_t show_attr_fan_rpm(struct class *class,
                                     struct class_attribute *attr,
                                     char *buf)
{
    char name[MAX_PATH_SIZE];
    int result = DEBUG_INV_INT;
    int present = DEBUG_INV_INT;

    memset(name, 0, MAX_PATH_SIZE);

	snprintf(name, MAX_PATH_SIZE, "%s", attr->attr.name);

    if ( strncmp(name, "fan1_front", strlen(name)) == 0 )
    {
        present = parsing_fan_present( HWMON_DEV_PATH_FAN_PRESENT, 0 );
        if ( present <= 0 )
            goto err_get_fan_module_rpm ;
        else
            result = read_fan_rpm(HWMON_DEV_PATH_FAN1_FRONT_RPM) ;
    }
    else if ( strncmp(name, "fan1_rear", strlen(name)) == 0 )
    {
        present = parsing_fan_present( HWMON_DEV_PATH_FAN_PRESENT, 0 );
        if ( present <= 0 )
            goto err_get_fan_module_rpm ;
        else
            result = read_fan_rpm(HWMON_DEV_PATH_FAN1_REAR_RPM) ;
    }
    else if ( strncmp(name, "fan2_front", strlen(name)) == 0 )
    {
        present = parsing_fan_present( HWMON_DEV_PATH_FAN_PRESENT, 1 );
        if ( present <= 0 )
            goto err_get_fan_module_rpm ;
        else
            result = read_fan_rpm(HWMON_DEV_PATH_FAN2_FRONT_RPM) ;
    }
    else if ( strncmp(name, "fan2_rear", strlen(name)) == 0 )
    {
        present = parsing_fan_present( HWMON_DEV_PATH_FAN_PRESENT, 1 );
        if ( present <= 0 )
            goto err_get_fan_module_rpm ;
        else
            result = read_fan_rpm(HWMON_DEV_PATH_FAN2_REAR_RPM) ;
    }
    else if ( strncmp(name, "fan3_front", strlen(name)) == 0 )
    {
        present = parsing_fan_present( HWMON_DEV_PATH_FAN_PRESENT, 2 );
        if ( present <= 0 )
            goto err_get_fan_module_rpm ;
        else
            result = read_fan_rpm(HWMON_DEV_PATH_FAN3_FRONT_RPM) ;
    }
    else if ( strncmp(name, "fan3_rear", strlen(name)) == 0 )
    {
        present = parsing_fan_present( HWMON_DEV_PATH_FAN_PRESENT, 2 );
        if ( present <= 0 )
            goto err_get_fan_module_rpm ;
        else
            result = read_fan_rpm(HWMON_DEV_PATH_FAN3_REAR_RPM) ;
    }
    else if ( strncmp(name, "fan4_front", strlen(name)) == 0 )
    {
        present = parsing_fan_present( HWMON_DEV_PATH_FAN_PRESENT, 3 );
        if ( present <= 0 )
            goto err_get_fan_module_rpm ;
        else
            result = read_fan_rpm(HWMON_DEV_PATH_FAN4_FRONT_RPM) ;
    }
    else if ( strncmp(name, "fan4_rear", strlen(name)) == 0 )
    {
        present = parsing_fan_present( HWMON_DEV_PATH_FAN_PRESENT, 3 );
        if ( present <= 0 )
            goto err_get_fan_module_rpm ;
        else
            result = read_fan_rpm(HWMON_DEV_PATH_FAN4_REAR_RPM) ;
    }
    else
    {
        goto err_get_fan_module_rpm ;
    }

    if ( result >= 0 )
        return snprintf(buf, MAX_PATH_SIZE, "%d RPM", result);
    else
        goto err_get_fan_module_rpm;

err_get_fan_module_rpm :
    return sprintf(buf, "N/A");
}

static struct class_attribute fan_class_attrs[] = {
	__ATTR(fan_nm,       S_IRUGO, show_attr_fan_number,  NULL),
	__ATTR(fan1_front,   S_IRUGO, show_attr_fan_rpm,     NULL),
	__ATTR(fan1_rear,    S_IRUGO, show_attr_fan_rpm,     NULL),
	__ATTR(fan2_front,   S_IRUGO, show_attr_fan_rpm,     NULL),
	__ATTR(fan2_rear,    S_IRUGO, show_attr_fan_rpm,     NULL),
	__ATTR(fan3_front,   S_IRUGO, show_attr_fan_rpm,     NULL),
	__ATTR(fan3_rear,    S_IRUGO, show_attr_fan_rpm,     NULL),
	__ATTR(fan4_front,   S_IRUGO, show_attr_fan_rpm,     NULL),
	__ATTR(fan4_rear,    S_IRUGO, show_attr_fan_rpm,     NULL),
	__ATTR(fan1_present, S_IRUGO, show_attr_fan_present, NULL),
	__ATTR(fan2_present, S_IRUGO, show_attr_fan_present, NULL),
	__ATTR(fan3_present, S_IRUGO, show_attr_fan_present, NULL),
	__ATTR(fan4_present, S_IRUGO, show_attr_fan_present, NULL),
	__ATTR_NULL,
};


static ssize_t show_attr_core_temperature(struct class *class,
                                     struct class_attribute *attr,
                                     char *buf)
{
    char name[MAX_PATH_SIZE];
    int result = DEBUG_INV_INT;

    memset(name, 0, MAX_PATH_SIZE);

	snprintf(name, MAX_PATH_SIZE, "%s", attr->attr.name);

    if ( strncmp(name, "core0", strlen(name)) == 0 )
        result = read_core_temperature(HWMON_DEV_PATH_CORE0_TEMP) ;
    else if ( strncmp(name, "core1", strlen(name)) == 0 )
        result = read_core_temperature(HWMON_DEV_PATH_CORE1_TEMP) ;
    else if ( strncmp(name, "core2", strlen(name)) == 0 )
        result = read_core_temperature(HWMON_DEV_PATH_CORE2_TEMP) ;
    else if ( strncmp(name, "core3", strlen(name)) == 0 )
        result = read_core_temperature(HWMON_DEV_PATH_CORE3_TEMP) ;
    else
        goto err_get_core_module_temperature ;

    if ( result == -1 )
        goto err_get_core_module_temperature;

    if ( result > 0 )
        return snprintf(buf, MAX_PATH_SIZE, "+%d.0 C", (result/1000));
    else
        return snprintf(buf, MAX_PATH_SIZE, "%d.0 C", (result/1000));

    goto err_get_core_module_temperature ;

err_get_core_module_temperature :
    return sprintf(buf, "N/A");
}

static struct class_attribute core_class_attrs[] = {
	__ATTR(core0, S_IRUGO, show_attr_core_temperature, NULL),
	__ATTR(core1, S_IRUGO, show_attr_core_temperature, NULL),
	__ATTR(core2, S_IRUGO, show_attr_core_temperature, NULL),
	__ATTR(core3, S_IRUGO, show_attr_core_temperature, NULL),
	__ATTR_NULL,
};

struct class core_class = {
	.name =		CORE_CLS_NAME,
	.owner =	THIS_MODULE,
	.class_attrs =	core_class_attrs,
};

struct class psu_class = {
	.name =		PSU_CLS_NAME,
	.owner =	THIS_MODULE,
	.class_attrs =	psu_class_attrs,
};

struct class fan_class = {
	.name =		FAN_CLS_NAME,
	.owner =	THIS_MODULE,
	.class_attrs =	fan_class_attrs,
};

int
core_init(void) {

    int status;

    /* Create core class object and class attribute */
	status = class_register(&core_class);
    core_class_p = &core_class;
    MONITOR_INFO("core_init result: %d\n", status);

	if (status < 0)
    {
        MONITOR_ERR("Inventec monitor core class creating failed.\n");
		return status;
    }

	return 0 ;
}

int
psu_init(void) {

    int status;

    /* Create psu class object and class attribute */
	status = class_register(&psu_class);
    psu_class_p = &psu_class;
    MONITOR_INFO("psu_init result: %d\n", status);

	if (status < 0)
    {
        MONITOR_ERR("Inventec monitor psu class creating failed.\n");
		return status;
	}

	/* Create psu device attribute */
	if ( register_psu_device() < 0 )
    {
        MONITOR_ERR("Inventec monitor psu device attribute creating failed.\n");
        return -1;
    }
	return 0 ;
}

int
fan_init(void) {

    int status;
    int index = 0 ;

    /* Create fan class object and class attribute */
	status = class_register(&fan_class);
    fan_class_p = &fan_class;
    MONITOR_INFO("fan_init result: %d\n", status);

	if (status < 0)
    {
        MONITOR_ERR("Inventec monitor fan class creating failed.\n");
		return status;
	}

	return 0 ;
}

static int __init monitor_sysfs_init(void)
{
	int status;

    if (inv_proxy_init() < 0) {
        MONITOR_ERR("Inventec monitor inv_proxy_init module initial failed.\n", MONITOR_VERSION);
    }

	if (core_init() < 0) {
		MONITOR_ERR("Inventec monitor core_init module initial failed.\n", MONITOR_VERSION);
	}

	if (psu_init() < 0) {
		MONITOR_ERR("Inventec monitor psu_init module initial failed.\n", MONITOR_VERSION);
	}

	if (fan_init() < 0) {
		MONITOR_ERR("Inventec monitor fan_init module initial failed.\n", MONITOR_VERSION);
	}

    MONITOR_INFO("Inventec monitor module V.%s initial success.\n", MONITOR_VERSION);
	return 0;
}

static void __exit monitor_sysfs_exit(void)
{
    class_unregister(core_class_p);
    class_unregister(fan_class_p);
    remove_psu_device();
    class_unregister(psu_class_p);
    unregister_chrdev_region(MKDEV(psu_major, 0), NUM_PSU);
    inv_proxy_exit();
    MONITOR_INFO("Inventec monitor module V.%s remove success.\n", MONITOR_VERSION);
    return 0;
}

module_init(monitor_sysfs_init);
module_exit(monitor_sysfs_exit);

/*  Module information  */
MODULE_AUTHOR(MONITOR_AUTHOR);
MODULE_DESCRIPTION(MONITOR_DESC);
MODULE_VERSION(MONITOR_VERSION);
MODULE_LICENSE(MONITOR_LICENSE);
