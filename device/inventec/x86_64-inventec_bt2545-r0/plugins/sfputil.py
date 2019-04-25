# sfputil.py
#
# Platform-specific SFP transceiver interface for SONiC
#

try:
    import time
    from sonic_sfp.sfputilbase import SfpUtilBase
except ImportError as e:
    raise ImportError("%s - required module not found" % str(e))


class SfpUtil(SfpUtilBase):
    """Platform-specific SfpUtil class"""

    PORT_START = 0
    PORT_END = 55
    PORTS_IN_BLOCK = 56
    QSFP_PORT_START = 48
    QSFP_PORT_END = 55

    _port_to_eeprom_mapping = {}
    port_to_i2c_mapping = {
        0:23,
        1:22,
        2:25,
        3:24,
        4:27,
        5:26,
        6:29,
        7:28,
        8:31,
        9:30,
        10:33,
        11:32,
        12:35,
        13:34,
        14:37,
        15:36,
        16:39,
        17:38,
        18:41,
        19:40,
        20:43,
        21:42,
        22:45,
        23:44,
        24:47,
        25:46,
        26:49,
        27:48,
        28:51,
        29:50,
        30:53,
        31:52,
        32:55,
        33:54,
        34:57,
        35:56,
        36:59,
        37:58,
        38:61,
        39:60,
        40:63,
        41:62,
        42:65,
        43:64,
        44:67,
        45:66,
        46:69,
        47:68,
        48:15,
        49:14,
        50:17,
        51:16,
        52:19,
        53:18,
        54:21,
        55:20

    }

    @property
    def port_start(self):
        return self.PORT_START

    @property
    def port_end(self):
        return self.PORT_END

    @property
    def qsfp_port_start(self):
        return self.QSFP_PORT_START

    @property
    def qsfp_port_end(self):
        return self.QSFP_PORT_END

    @property
    def qsfp_ports(self):
        return range(self.QSFP_PORT_START, self.PORTS_IN_BLOCK + 1)

    @property
    def port_to_eeprom_mapping(self):
        return self._port_to_eeprom_mapping

    def __init__(self):
        eeprom_path = "/sys/bus/i2c/devices/{0}-0050/eeprom"

        for x in range(0, self.port_end + 1):
            port_eeprom_path = eeprom_path.format(self.port_to_i2c_mapping[x])
            self.port_to_eeprom_mapping[x] = port_eeprom_path
        SfpUtilBase.__init__(self)

    def get_presence(self, port_num):
        # Check for invalid port_num
        if port_num < self.port_start or port_num > self.port_end:
            return False

        try:
            reg_file = open("/sys/class/swps/port"+str(port_num+1)+"/present")
        except IOError as e:
            print "Error: unable to open file: %s" % str(e)
            return False

        reg_value = int(reg_file.readline().rstrip())

        if reg_value == 0:
            return True

        return False

    def get_low_power_mode(self, port_num):
        # Check for invalid port_num
        if port_num < self.port_start or port_num > self.port_end:
            return False
        if port_num < self.qsfp_port_start or port_num > self.qsfp_port_end:
            return False

        try:
            reg_file = open("/sys/class/swps/port"+str(port_num)+"/lpmod")
        except IOError as e:
            print "Error: unable to open file: %s" % str(e)

        reg_value = int(reg_file.readline().rstrip())

        if reg_value == 0:
            return False

        return True

    def set_low_power_mode(self, port_num, lpmode):
        # Check for invalid port_num
        if port_num < self.port_start or port_num > self.port_end:
            return False
        if port_num < self.qsfp_port_start or port_num > self.qsfp_port_end:
            print "\nError:SFP's don't support this property"
            return False

        try:
            reg_file = open("/sys/class/swps/port"+str(port_num)+"/lpmod", "r+")
        except IOError as e:
            print "Error: unable to open file: %s" % str(e)
            return False

        reg_value = int(reg_file.readline().rstrip())

        # LPMode is active high; set or clear the bit accordingly
        if lpmode is True:
            reg_value = 1
        else:
            reg_value = 0

        reg_file.write(hex(reg_value))
        reg_file.close()

        return True

    def reset(self, port_num):
        QSFP_RESET_REGISTER_DEVICE_FILE = "/sys/class/swps/port"+str(port_num)+"/reset"
        # Check for invalid port_num
        if port_num < self.port_start or port_num > self.port_end:
            return False
        if port_num < self.qsfp_port_start or port_num > self.qsfp_port_end:
            print "\nError:SFP's don't support this property"
            return False

        try:
            reg_file = open(QSFP_RESET_REGISTER_DEVICE_FILE, "r+")
        except IOError as e:
            print "Error: unable to open file: %s" % str(e)
            return False

        reg_value = 0
        reg_file.write(hex(reg_value))
        reg_file.close()

        # Sleep 2 second to allow it to settle
        time.sleep(2)

        # Flip the value back write back to the register to take port out of reset
        try:
            reg_file = open(QSFP_RESET_REGISTER_DEVICE_FILE, "r+")
        except IOError as e:
            print "Error: unable to open file: %s" % str(e)
            return False

        reg_value = 1
        reg_file.write(hex(reg_value))
        reg_file.close()

        return True

    def get_transceiver_change_event(self):
        """
        TODO: This function need to be implemented
        """
        raise NotImplementedError
