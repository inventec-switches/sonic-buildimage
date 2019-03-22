#ifndef _INV_PROXY_H_
#define _INV_PROXY_H_

#define MAX_ACC_SIZE             (1023)
#define MAX_PATH_SIZE (64)

/* Alarm define */
#define ALARM_ASIC_WARN          (90)
#define ALARM_CPU_WARN           (88)
#define ALARM_ENV_WARN           (57)

/* Val */
#define VAL_SFP_ID               (0x03)
#define VAL_QSFP_ID              (0x0c)
#define VAL_QSFP_P_ID            (0x0d)
#define VAL_QSFP_28_ID           (0x11)
#define VAL_VPD_CHANNEL          (0)
#define VAL_VPD_ADDRESS          (0x53)
#define VAL_CPLD_CHANNEL         (0)
#define VAL_CPLD_ADDRESS         (0x55)
#define VAL_SYSLED_MOD_OFFSET    (0x0c)
#define VAL_SYSLED_MOD_AUTO      "0"     /* CPLD_VAL: 0x10 */
#define VAL_SYSLED_MOD_MANUAL    "1"     /* CPLD_VAL: 0x11 */
#define VAL_SYSLED_INV_KEEP_ON   (7)
#define VAL_SYSLED_INV_4_HZ      (4)
#define VAL_SYSLED_INV_2_HZ      (3)
#define VAL_SYSLED_INV_1_HZ      (2)
#define VAL_SYSLED_INV_HALF_HZ   (1)
#define VAL_SYSLED_INV_OFF       (0)
#define VAL_BIOS_MASTER          (0x0)
#define VAL_BIOS_SLAVER          (0x1)

/* For uevent attribute key */
#define UKEY_DEV_TYPE             "type"

/* Private value */
#define MAX_LEN_TRNASFORM_W_TMP  (128)
#define MAX_LEN_VPD_ATTR         (32)
#define MAX_LEN_SHOW_NUMBER_BUF  (16)
#define MAX_LEN_SHOW_STR_BUF     (32)
#define MAX_LEN_SHOW_LSTR_BUF    (64)
#define MAX_LEN_SHOW_XLSTR_BUF   (128)

#define DEBUG_INV_HEX            (0xfe)
#define DEBUG_INV_INT            (-99)
#define DEBUG_INV_STR            "INV_DEBUG"

/* Private error code */
#define ERR_NOT_OWNER            (-10000)
#define ERR_NEGATIVE_VAL         (-10001)
#define ERR_ZERO_VAL             (-10002)
#define ERR_NO_CTLER             (-10003)

/* VPD info */
#define ATTR_VPD_FILE_PATH       "/tmp/vpd.txt"
//#define ATTR_VPD_VNEDOR_NAME     "Vendor Name"
#define ATTR_VPD_PRODUCT_NAME    "Product Name"
#define ATTR_VPD_SERIAL_NUM      "Serial Number"
#define ATTR_VPD_VERSION         "Device Version"
#define ATTR_VPD_BASE_MAC        "Base MAC Address"
#define ATTR_VPD_DATE            "Manufacture Date"


#define PROXY_INFO(fmt, args...) printk( KERN_INFO "[Proxy]: " fmt, ##args)
#define PROXY_ERR(fmt, args...)  printk( KERN_ERR  "[Proxy]: " fmt, ##args)

#ifdef DEBUG_PROXY
#   define PROXY_DEBUG(fmt, args...) printk( KERN_DEBUG "[Proxy]: " fmt, ##args)
#else
#   define PROXY_DEBUG(fmt, args...)
#endif /*DEBUG*/


typedef enum {
    INV_PSU_NORMAL       = 0,
    INV_PSU_UNPOWERED    = 2,
    INV_PSU_FAULT        = 4,
    INV_PSU_UNINSTALLED  = 7,
}inv_psu_state_t;

struct vpd_info_s {
    char vendor_name[MAX_LEN_VPD_ATTR];
    char product_name[MAX_LEN_VPD_ATTR];
    char serial_num[MAX_LEN_VPD_ATTR];
    char version[MAX_LEN_VPD_ATTR];
    char cpu_mac[MAX_LEN_VPD_ATTR];
    char switch_mac[MAX_LEN_VPD_ATTR];
    char date[MAX_LEN_VPD_ATTR];
};

extern struct vpd_info_s * get_vpd_info(void);
extern char* str_replace(char* str, char* search, char* replace);
extern void  str_line_to_var(char *str);
extern void  set_psco_io_flag(int flag);
extern int get_psco_io_flag(void);
extern int sscanf_2_int(const char *buf);
extern int is_ufile_exist(const char *path);
extern int transform_4_read_binary(char *buf, int buf_len, int err);
extern int transform_4_read_uint8(char *buf, int buf_len, int err);
extern int transform_4_read_int(char *buf, int buf_len, int err);
extern int transform_4_read_positive(char *buf, int buf_len, int err);
extern int transform_4_read_str(char *buf, int buf_len, int err);
extern int transform_4_write_binary(const char *buf, char *tmp);
extern int transform_4_write_uint8(const char *buf, char *tmp);
extern int transform_4_write_int(const char *buf, char *tmp);
extern int transform_4_read_percent(char *buf, int buf_len, int err);
extern int transform_4_write_percent(const char *buf, char *tmp);
extern int inv_proxy_init(void);
extern int inv_proxy_exit(void);
extern unsigned long get_jiffies_after_ms(int ms);
extern int read_hwmon_attr(char *attr, char *buf, int buf_len, int (*transform)(char *buf, int buf_len,  int err));
extern int read_psu_temperature(char *attr);
extern int read_psu_voltage(char *attr);
extern int read_psu_power(char *attr);
extern int read_psu_present(char *attr);
extern int read_fan_rpm(char *attr);
extern int read_fan_present(char *attr);
extern int read_core_temperature(char *attr);
#endif







