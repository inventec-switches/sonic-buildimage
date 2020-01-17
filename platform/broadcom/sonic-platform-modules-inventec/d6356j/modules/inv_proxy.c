#include <linux/module.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/err.h>
#include "inv_proxy.h"

static DEFINE_MUTEX(psoc_lock);
static int flag_psoc_io = 1;

struct vpd_info_s *vpd = NULL;


unsigned long
get_jiffies_after_ms(int ms) {

    int ms_jiffies = 0;

    if (ms == 0) {
        return jiffies;
    }
    ms_jiffies = ((ms * HZ) / 1000);
    if (ms_jiffies == 0) {
        ms_jiffies = 1;
    }
    return (jiffies + ms_jiffies);
}



char* str_replace(char* str, char* search, char* replace)
{
    char  *buf   = DEBUG_INV_STR;
    char  *pos   = DEBUG_INV_STR;
    char  *in    = DEBUG_INV_STR;
    char  *out   = DEBUG_INV_STR;
    long   slen  = strlen(search);
    long   rlen  = strlen(replace);
    size_t count = 0;
    size_t buf_size = 0;

    /* Calculate the frequency  */
    pos = str;
    while((pos = strstr(pos, search)))
    {
        count += 1;
        pos += slen;
    }
    /* Calculate size of new string  */
    buf_size = (long)strlen(str) + count*(rlen - slen) + 1;

    /* Copy and replace */
    buf = kzalloc(buf_size, GFP_KERNEL);
    if(!buf) {
        return NULL;
    }
    pos = in = str;
    out = buf;
    while((pos = strstr(pos, search)))
    {
        strncpy(out, in, pos - in);
        out += pos - in;
        strcpy(out, replace);
        out += rlen;
        pos += slen;
        in = pos;
    }
    strcpy(out, in);
    return buf;
}


void
str_line_to_var(char *str) {

    char *mark = NULL;
    mark = strpbrk(str, "\n");
    if (mark)
        *mark = '\0';
}


int
sscanf_2_int(const char *buf) {

    int   result  = -EBFONT;
    char *hex_tag = "0x";

    if (strcspn(buf, hex_tag) == 0) {
        if (sscanf(buf,"%x",&result)) {
            return result;
        }
    } else {
        if (sscanf(buf,"%d",&result)) {
            return result;
        }
        if (sscanf(buf,"%x",&result)) {
            return result;
        }
    }
    return -EBFONT;
}


int
is_ufile_exist(const char *path) {

    struct file *fp = filp_open(path, O_RDONLY, S_IRUGO);
    if (IS_ERR(fp))
        return -1;
    filp_close(fp, NULL);
    return 0;

}


int
read_ufile(const char *path,
           char  *buf,
           size_t len) {

    struct file *fp;
    mm_segment_t fs;
    loff_t pos;

    if (len >= MAX_ACC_SIZE) {
        len = MAX_ACC_SIZE - 1;
    }
    fp = filp_open(path, O_RDONLY, S_IRUGO);
    if (IS_ERR(fp))
        return -ENODEV;
    fs = get_fs();
    set_fs(KERNEL_DS);
    pos = 0;
    vfs_read(fp, buf, len, &pos);
    filp_close(fp, NULL);
    set_fs(fs);
    return 0;
}


int
write_ufile(const char *path,
            char  *buf,
            size_t len) {

    struct file *fp;
    mm_segment_t fs;
    loff_t pos;

    if (len >= MAX_ACC_SIZE) {
        len = MAX_ACC_SIZE - 1;
    }
    fp = filp_open(path, O_WRONLY, S_IWUSR | S_IRUGO);
    if (IS_ERR(fp))
        return -ENODEV;
    fs = get_fs();
    set_fs(KERNEL_DS);
    pos = 0;
    vfs_write(fp, buf, len, &pos);
    filp_close(fp, NULL);
    set_fs(fs);
    return 0;
}


int
_is_hwmon_errno(char *result) {

#if 0
    char *str_eio      = "-5\n";
    char *str_enxio    = "-6\n";
    char *str_negative = "-";
    char *str_no_ctl   = "64250\n"; /* 0xFAFA */

    if (strcmp(result, str_eio) == 0) {
        return -EIO;
    }
    if (strcmp(result, str_enxio) == 0) {
        return -ENXIO;
    }
    if (strcmp(result, str_no_ctl) == 0) {
        return ERR_NO_CTLER;
    }
    if (strcspn(result, str_negative) == 0) {
        /* error case: <ex> "-202" */
        return ERR_NEGATIVE_VAL;
    }
#endif
    return 0;
}


int
_is_swps_errno(char *result) {

    char *str_negative = "-";

    if (strcspn(result, str_negative) == 0) {
        /* error case: <ex> "-202" */
        return -ENODATA;
    }
    return 0;
}


int
_read_attr_common(char *path,
                  char *buf,
                  int   buf_len,
                  int (*check_err)(char* buf),
                  int (*transform)(char *buf, int buf_len, int err) ){

    int err     = DEBUG_INV_INT;
    int acc_len = MAX_ACC_SIZE-1;

    memset(buf, 0, buf_len);
    err = read_ufile(path, buf, acc_len);
    if (err < 0) {
        return err;
    }
    err = check_err(buf);
    if (err < 0) {
        return err;
    }
    err = transform(buf, buf_len, err);
    if (err < 0){
        return err;
    }
    return err;
}


int
_write_attr_common(char *path,
                   const char *buf,
                   int (*transform)(const char *buf, char *tmp) ){

    int  err     = DEBUG_INV_INT;
    int  acc_len = MAX_ACC_SIZE-1;
    char tmp[MAX_LEN_TRNASFORM_W_TMP] = DEBUG_INV_STR;

    memset(tmp,  0, MAX_LEN_TRNASFORM_W_TMP);
    err = transform(buf, tmp);
    if (err < 0) {
        return err;
    }
    err = write_ufile(path, tmp, acc_len);
    if (err < 0){
        return err;
    }
    return 0;
}


void
set_psco_io_flag(int flag) {

    /* IO will be disabled if flag = 0 */
    mutex_lock(&psoc_lock);
    flag_psoc_io = flag;
    mutex_unlock(&psoc_lock);
}


int
get_psco_io_flag(void) {

    int ret = 0;

    mutex_lock(&psoc_lock);
    ret = flag_psoc_io;
    mutex_unlock(&psoc_lock);
    return ret;
}

int
transform_4_read_uint8(char *buf,
                       int buf_len,
                       int err) {

    int limit  = 32;
    int result = DEBUG_INV_HEX;

    if (err < 0) {
        goto err_transform_4_read_uint8;
    }
    if (sscanf(buf, "%x", &result) != 1) {
        err = -EBFONT;
        goto err_transform_4_read_uint8;
    }
    if (result == DEBUG_INV_HEX) {
        err = -EBFONT;
        goto err_transform_4_read_uint8;
    }
    memset(buf, 0, buf_len);
    snprintf(buf, limit, "%02x  \n", result);
    return 0;

err_transform_4_read_uint8:
    snprintf(buf, limit, "%d\n", err);
    return err;
}


int
transform_4_read_int(char *buf,
                     int buf_len,
                     int err) {

    int limit  = 32;
    int result = DEBUG_INV_INT;

    if (err < 0) {
        goto err_transform_4_read_int;
    }
    if (sscanf(buf, "%d", &result) != 1) {
        err = -EBFONT;
        if( sscanf(buf,"-%d",&result) == 1 ){
            result = -result;
            err = 0;
        }
        else
            goto err_transform_4_read_int;
    }
    if (result == DEBUG_INV_INT) {
        err = -EBFONT;
        goto err_transform_4_read_int;
    }
    memset(buf, 0, buf_len);
    snprintf(buf, limit, "%d\n", result);
    return 0;

err_transform_4_read_int:
    snprintf(buf, limit, "%d\n", err);
    return err;
}


int
transform_4_read_percent(char *buf,
                     int buf_len,
                     int err) {

    int limit  = 32;
    int result = DEBUG_INV_INT;
    int iErrHandle = 0;

    if (err < 0) {
        iErrHandle = 1;
    }
    if (sscanf(buf, "%d", &result) != 1) {
        err = -EBFONT;
        iErrHandle = 1;
    }
    if (result == DEBUG_INV_INT) {
        err = -EBFONT;
        iErrHandle = 1;
    }
    memset(buf, 0, buf_len);
    if( !iErrHandle ){
        result = (result * 100 )/255;
        snprintf(buf, limit, "%d\n", result);
        return 0;
    }
    else{
        snprintf(buf, limit, "%d\n", err);
        return err;
    }
}

int
transform_4_read_positive(char *buf,
                          int buf_len,
                          int err) {

    int result = DEBUG_INV_INT;
    int check  = transform_4_read_int(buf, buf_len, err);

    if (check < 0) {
        return check;
    }
    if (sscanf(buf, "%d", &result) != 1){
        return -EBFONT;
    }
    if (result < 0) {
        return ERR_NEGATIVE_VAL;
    }
    if (result == 0) {
        return ERR_ZERO_VAL;
    }
    return 0;
}


int
transform_4_read_binary(char *buf,
                        int buf_len,
                        int err) {

    int result = DEBUG_INV_INT;
    int check  = transform_4_read_int(buf, buf_len, err);

    if (check < 0) {
        return check;
    }
    if (sscanf(buf, "%d", &result) != 1){
        return -EBFONT;
    }
    if ((result == 0) || (result == 1)) {
        return 0;
    }
    return - ERANGE;
}


int
transform_4_read_str(char *buf,
                     int buf_len,
                     int err) {

    if (err < 0) {
        goto err_transform_4_read_str;
    }
    return 0;

err_transform_4_read_str:
    snprintf(buf, buf_len, "%d\n", err);
    return err;
}


int
transform_4_write_binary(const char *buf, char *tmp) {

    int input = sscanf_2_int(buf);

    if (input < 0) {
        return input;
    }
    if ( (input == 0) || (input == 1) ) {
        goto ok_transform_4_write_binary;
    }
    return -EBFONT;

ok_transform_4_write_binary:
    memset(tmp, 0, MAX_LEN_TRNASFORM_W_TMP);
    snprintf(tmp, MAX_LEN_TRNASFORM_W_TMP, "%d", input);
    return 0;
}


int
transform_4_write_uint8(const char *buf, char *tmp) {

    int input = sscanf_2_int(buf);

    if (input < 0) {
        return input;
    }
    if ((input >= 0x0) && (input <= 0xff)) {
        goto ok_transform_4_write_uint8;
    }
    return -EBFONT;

ok_transform_4_write_uint8:
    memset(tmp, 0, MAX_LEN_TRNASFORM_W_TMP);
    snprintf(tmp, MAX_LEN_TRNASFORM_W_TMP, "%d", input);
    return 0;
}
int
transform_4_write_percent(const char *buf, char *tmp) {

    int input = sscanf_2_int(buf);

    if (input < 0) {
        return input;
    }
    if ((input >= 0) && (input <= 100)) {
        input = (input *255)/100;
        memset(tmp, 0, MAX_LEN_TRNASFORM_W_TMP);
        snprintf(tmp, MAX_LEN_TRNASFORM_W_TMP, "%d", input);
        return 0;
    }
    return -EBFONT;

}


int
transform_4_write_int(const char *buf, char *tmp) {

    int input;

    if (sscanf(buf, "%d", &input) == 1){
        goto ok_transform_4_write_int;
    }
    if (sscanf(buf, "%x", &input) == 1){
        goto ok_transform_4_write_int;
    }
    return -EBFONT;

ok_transform_4_write_int:
    memset(tmp, 0, MAX_LEN_TRNASFORM_W_TMP);
    snprintf(tmp, MAX_LEN_TRNASFORM_W_TMP, "%d", input);
    return 0;
}


static int
_count_switch_mac(char *base_mac,
                  char *switch_mac,
                  int level) {

    int i, ret;
    int num[6];
    char *delim = ":";
    char *tmp_str = NULL;
    char *tmp_base = NULL;
    char  tmp_mac[MAX_LEN_VPD_ATTR] = DEBUG_INV_STR;

    if ((level < 0) || (level > 5)) {
        goto err_count_switch_mac_1;
    }
    tmp_base = kzalloc(MAX_LEN_VPD_ATTR, GFP_KERNEL);
    if (!tmp_base) {
        goto err_count_switch_mac_1;
    }
    memset(switch_mac, 0, MAX_LEN_VPD_ATTR);
    memset(tmp_mac, 0, MAX_LEN_VPD_ATTR);
    snprintf(tmp_base, 32, "%s", base_mac);
    for (i=0; i<6; i++) {
        tmp_str = NULL;
        tmp_str = strsep(&tmp_base, delim);
        if (!tmp_str) {
            goto err_count_switch_mac_2;
        }
        if (!sscanf(tmp_str,"%x",&(num[i]))) {
            goto err_count_switch_mac_2;
        }
    }
    if (num[level] < 0xff) {
        num[level] += 1;
        snprintf(switch_mac, 32, "%02x:%02x:%02x:%02x:%02x:%02x",
                num[0], num[1], num[2], num[3], num[4], num[5]);
        ret = 0;
        goto ok_count_switch_mac_1;
    } else if (num[level] == 0xff) {
        num[level] = 0;
        level -= 1;
        snprintf(tmp_mac, 32, "%02x:%02x:%02x:%02x:%02x:%02x",
                num[0], num[1], num[2], num[3], num[4], num[5]);
        ret = _count_switch_mac(tmp_mac, switch_mac, level);
        goto ok_count_switch_mac_1;
    }
    goto err_count_switch_mac_2;

ok_count_switch_mac_1:
    if (tmp_base) {
        kfree(tmp_base);
    }
    return ret;

err_count_switch_mac_2:
    if (tmp_base) {
        kfree(tmp_base);
    }
err_count_switch_mac_1:
    return -1;
}


static int
_parser_vpd_attr_str(char *line, char *val) {

    int i, off;
    int len_result = 0;
    int len_ignore = 30;

    if (strlen(line) < len_ignore) {
        goto err_parser_vpd_attr_str;
    }
    len_result = strlen(line) - len_ignore;
    if (len_result < strlen(val)) {
        PROXY_ERR("detect redundance attribute in VPD.");
        goto err_parser_vpd_attr_str;
    }
    memset(val, 0, MAX_LEN_VPD_ATTR);
    for (i=0; i<=len_result; i++) {
        off = i + len_ignore;
        if ( (line[off] == '\n') ||
             (line[off] == '\0') ||
             (i == (len_result)) ){
            val[i] = '\0';
            break;
        }
        val[i] = line[off];
    }
    return 0;

err_parser_vpd_attr_str:
    snprintf(val, MAX_LEN_VPD_ATTR, "NA");
    return -1;
}


static int
_parser_vpd_attr_mac(char *line,
                     char *cpu_mac,
                     char *switch_mac) {

    int err = _parser_vpd_attr_str(line, cpu_mac);

    if (err < 0) {
        PROXY_ERR("%s: Parser CPU MAC fail\n", __func__);
        return -1;
    }
    err = _count_switch_mac(cpu_mac, switch_mac, 5);
    if (err < 0) {
        PROXY_ERR("%s: Count Switch MAC fail\n", __func__);
        return -1;
    }
    return 0;
}


static int
_setup_vpd_attr(char *line) {

    if (strncmp(line, ATTR_VPD_PRODUCT_NAME, strlen(ATTR_VPD_PRODUCT_NAME)) == 0) {
        return _parser_vpd_attr_str(line, vpd->product_name);
    }
    if (strncmp(line, ATTR_VPD_SERIAL_NUM, strlen(ATTR_VPD_SERIAL_NUM)) == 0) {
        return _parser_vpd_attr_str(line, vpd->serial_num);
    }
    if (strncmp(line, ATTR_VPD_VERSION, strlen(ATTR_VPD_VERSION)) == 0) {
        return _parser_vpd_attr_str(line, vpd->version);
    }
    if (strncmp(line, ATTR_VPD_BASE_MAC, strlen(ATTR_VPD_BASE_MAC)) == 0) {
        return _parser_vpd_attr_mac(line, vpd->cpu_mac, vpd->switch_mac);
    }
    if (strncmp(line, ATTR_VPD_DATE, strlen(ATTR_VPD_DATE)) == 0) {
        return _parser_vpd_attr_str(line, vpd->date);
    }
    return 0;
}


static int
_read_vpd_attr(char *buf) {

    int ret = 0;
    const char *delim = "\n";
    char *vender_name = "Inventec";
    char *line = strsep(&buf, delim);

    while (line != NULL) {
        if (_setup_vpd_attr(line) < 0) {
            ret = -1;
        }
        line = strsep(&buf, delim);
    }
    snprintf(vpd->vendor_name, 32, "%s", vender_name);
    return ret;
}


static int
_read_vpd_info(void) {

    int  err = DEBUG_INV_INT;
    char buf[MAX_ACC_SIZE] = DEBUG_INV_STR;

    memset(buf, 0, sizeof(buf));
    err = read_ufile(ATTR_VPD_FILE_PATH, buf, MAX_ACC_SIZE);
    if (err < 0) {
        PROXY_DEBUG("%s: IO fail. <err>:%d\n", __func__, err);
        return err;
    }
    return _read_vpd_attr(buf);
}


struct vpd_info_s *
get_vpd_info(void) {

    int  i       = 0;
    int  retry   = 5;
    int  wait_ms = 100;
    int  err     = DEBUG_INV_INT;

    if (vpd) {
        return vpd;
    }
    vpd = kzalloc(sizeof(struct vpd_info_s), GFP_KERNEL);
    if (!vpd) {
        PROXY_ERR("%s: kzalloc fail. <err>:%d\n", __func__, err);
        return NULL;
    }
    while (i++ < retry) {
        if (_read_vpd_info() == 0) {
            break;
        }
        if (i >= retry) {
            PROXY_ERR("%s: read VPD fail!\n", __func__);
            kfree(vpd);
            vpd = NULL;
            return NULL;
        }
        mdelay(wait_ms);
    }
    PROXY_DEBUG("Get vendor_name  : %s\n", vpd->vendor_name);
    PROXY_DEBUG("Get product_name : %s\n", vpd->product_name);
    PROXY_DEBUG("Get serial_num   : %s\n", vpd->serial_num);
    PROXY_DEBUG("Get version      : %s\n", vpd->version);
    PROXY_DEBUG("Get cpu_mac      : %s\n", vpd->cpu_mac);
    PROXY_DEBUG("Get switch_mac   : %s\n", vpd->switch_mac);
    PROXY_DEBUG("Get date         : %s\n", vpd->date);
    return vpd;
}


struct i2c_client *
_get_i2c_client(int channel) {

    struct i2c_adapter *adap  = NULL;
    struct i2c_client *client = NULL;

    adap = i2c_get_adapter(VAL_CPLD_CHANNEL);
    if(!adap) {
        PROXY_ERR("%s: Can't get i2c adap.\n", __func__);
        return NULL;
    }
    client = kzalloc(sizeof(*client), GFP_KERNEL);
    if (!client){
        PROXY_ERR("%s: kzalloc client fail!\n", __func__);
        return NULL;
    }
    client->adapter = adap;
    return client;
}

/* HWMON */
int
read_hwmon_attr(char *attr,
               char *buf,
               int   buf_len,
               int (*transform)(char *buf, int buf_len, int err))
{

    int  ret = -EIO;
    char path[64] = DEBUG_INV_STR;
    memset(path, 0, sizeof(path));

    snprintf(path, sizeof(path), "%s", attr);
    ret = _read_attr_common(path,
                            buf,
                            buf_len,
                            _is_hwmon_errno,
                            transform);
}

/* PSU */

int
read_psu_temperature(char *attr) {

    char tmp[MAX_PATH_SIZE];
    int result = DEBUG_INV_INT;
    memset(tmp, 0, MAX_PATH_SIZE);

    if (read_hwmon_attr(attr,
                       tmp,
                       MAX_PATH_SIZE,
                       transform_4_read_int) < 0)
    {
        PROXY_ERR("%s: use inv proxy to read path (%s)\n", __func__, attr);
        memset(tmp, 0, MAX_PATH_SIZE);
        return -1;
    }
    if (sscanf(tmp, "%d", &result) != 1)
    {
        if (sscanf(tmp, "-%d", &result) == 1)
        {
            result = -result;
        }
    }
    return result;
}

int
read_psu_voltage(char *attr) {

    char tmp[MAX_PATH_SIZE];
    int result = DEBUG_INV_INT;
    memset(tmp, 0, MAX_PATH_SIZE);

    if (read_hwmon_attr(attr,
                       tmp,
                       MAX_PATH_SIZE,
                       transform_4_read_int) < 0)
    {
        PROXY_ERR("%s: use inv proxy to read path (%s)\n", __func__, attr);
        memset(tmp, 0, MAX_PATH_SIZE);
        return -1;
    }
    if (sscanf(tmp, "%d", &result) != 1)
    {
        if (sscanf(tmp, "-%d", &result) == 1)
        {
            result = -result;
        }
    }
    return result;
}

int
read_psu_power(char *attr) {

    char tmp[MAX_PATH_SIZE];
    int result = DEBUG_INV_INT;
    memset(tmp, 0, MAX_PATH_SIZE);

    if (read_hwmon_attr(attr,
                       tmp,
                       MAX_PATH_SIZE,
                       transform_4_read_int) < 0)
    {
        PROXY_ERR("%s: use inv proxy to read path (%s)\n", __func__, attr);
        memset(tmp, 0, MAX_PATH_SIZE);
        return -1;
    }
    if (sscanf(tmp, "%d", &result) != 1)
    {
        if (sscanf(tmp, "-%d", &result) == 1)
        {
            result = -result;
        }
    }
    return result;
}

int
read_psu_present(char *attr) {

    char tmp[MAX_PATH_SIZE];
    int result = DEBUG_INV_INT;
    memset(tmp, 0, MAX_PATH_SIZE);

    if (read_hwmon_attr(attr,
                       tmp,
                       MAX_PATH_SIZE,
                       transform_4_read_str) < 0)
    {
        PROXY_ERR("%s: use inv proxy to read path (%s)\n", __func__, attr);
        memset(tmp, 0, MAX_PATH_SIZE);
        return -1;
    }
    if (sscanf(tmp, "%d:", &result) != 1)
    {
        return -1;
    }
    // Default setting from kernel hw driver
    // 0   menas "Unpowered"
    // 1   means "Normal"
    // 2,3 means "Uninstall"
    // For the consistency, we will swap them.
    // 0   means "Uninstall"
    // 1   means "Normal"
    // 2   means "Installed but Unpowered"

    if ( result == 0 )
        return 2 ;
    if ( result == 2 || result == 3 )
        return 0 ;
    return result;
}

/* FAN */

int
read_fan_rpm(char *attr) {

    char tmp[MAX_PATH_SIZE];
    int result = DEBUG_INV_INT;
    memset(tmp, 0, MAX_PATH_SIZE);

    if (read_hwmon_attr(attr,
                       tmp,
                       MAX_PATH_SIZE,
                       transform_4_read_int) < 0)
    {
        PROXY_ERR("%s: use inv proxy to read path (%s)\n", __func__, attr);
        memset(tmp, 0, MAX_PATH_SIZE);
        return -1;
    }
    if (sscanf(tmp, "%d", &result) != 1)
    {
        return -1;
    }
    return result;
}

int
read_fan_present(char *attr) {

    char tmp[MAX_PATH_SIZE];
    int result = DEBUG_INV_INT;
    memset(tmp, 0, MAX_PATH_SIZE);

    if (read_hwmon_attr(attr,
                       tmp,
                       MAX_PATH_SIZE,
                       transform_4_read_str) < 0)
    {
        PROXY_ERR("%s: use inv proxy to read path (%s)\n", __func__, attr);
        memset(tmp, 0, MAX_PATH_SIZE);
        return -1;
    }
    if (sscanf(tmp, "%d:", &result) != 1)
    {
        return -1;
    }
    // Default setting from kernel hw driver
    // 0   menas "Normal type"
    // 1   means "Reversal type"
    // 2,3 means "Unplugged"
    // For the consistency, we will swap them.
    // 0   means "Unplugged"
    // 1   means "Normal type"
    // 2   means "Reversal type"
    if ( result == 0 )
        return 1 ;
    if ( result == 1 )
        return 2 ;
    if ( result == 2 || result == 3 )
        return 0 ;
    return result;
}

/* CORE */

int
read_core_temperature(char *attr) {

    char tmp[MAX_PATH_SIZE];
    int result = DEBUG_INV_INT;
    memset(tmp, 0, MAX_PATH_SIZE);

    if (read_hwmon_attr(attr,
                       tmp,
                       MAX_PATH_SIZE,
                       transform_4_read_int) < 0)
    {
        PROXY_ERR("%s: use inv proxy to read path (%s)\n", __func__, attr);
        memset(tmp, 0, MAX_PATH_SIZE);
        return -1 ;
    }
    if (sscanf(tmp, "%d", &result) != 1)
    {
        if (sscanf(tmp, "-%d", &result) == 1)
        {
            result = -result;
        }
    }
    return result;
}

int
inv_proxy_init(void) {

    if (vpd) {
        kfree(vpd);
    }
    vpd = NULL;
    return 0;
}


int
inv_proxy_exit(void) {

    if (vpd) {
        kfree(vpd);
    }
    return 0;
}

