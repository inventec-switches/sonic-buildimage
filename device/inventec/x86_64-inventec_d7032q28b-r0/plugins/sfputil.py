# sfputil.py
#
# Platform-specific SFP transceiver interface for SONiC
#

try:
    import time
    import socket, re,os
    from collections import OrderedDict
    from sonic_sfp.sfputilbase import SfpUtilBase
    from sonic_sfp.sff8436 import sff8436DomThreshold
    from sonic_sfp.sff8436 import sff8436InterfaceId
    from sonic_sfp.sff8436 import sff8436Dom
    from sonic_sfp.sff8472 import sff8472InterfaceId
    from sonic_sfp.sff8472 import sff8472Dom
except ImportError as e:
    raise ImportError("%s - required module not found" % str(e))

NETLINK_KOBJECT_UEVENT = 15
monitor = None

class SWPSEventMonitor(object):

    def __init__(self):
        self.recieved_events = OrderedDict()
        self.socket = socket.socket(
            socket.AF_NETLINK, socket.SOCK_DGRAM, NETLINK_KOBJECT_UEVENT)

    def start(self):
        self.socket.bind((os.getpid(), -1))

    def stop(self):
        self.socket.close()

    def __enter__(self):
        self.start()
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.stop()

    def __iter__(self):
        global monitor
        while True:
            for item in monitor.next_events():
                yield item

    def next_events(self):
        data = self.socket.recv(16384)
        event = {}
        for item in data.split(b'\x00'):
            if not item:
                # check if we have an event and if we already received it
                if event and event['SEQNUM'] not in self.recieved_events:
                    self.recieved_events[event['SEQNUM']] = None
                    if (len(self.recieved_events) > 100):
                        self.recieved_events.popitem(last=False)
                    yield event
                event = {}
            else:
                try:
                    k, v = item.split(b'=', 1)
                    event[k.decode('ascii')] = v.decode('ascii')
                except ValueError:
                    pass

class SfpUtil(SfpUtilBase):
    """Platform-specific SfpUtil class"""

    PORT_START = 0
    PORT_END = 31
    PORTS_IN_BLOCK = 32
    QSFP_PORT_START = 0
    QSFP_PORT_END = 31

    _port_to_eeprom_mapping = {}
    port_to_i2c_mapping = {
        0: 22,
        1: 23,
        2: 24,
        3: 25,
        4: 26,
        5: 27,
        6: 28,
        7: 29,
        8: 30,
        9: 31,
        10: 32,
        11: 33,
        12: 34,
        13: 35,
        14: 36,
        15: 37,
        16: 6,
        17: 7,
        18: 8,
        19: 9,
        20: 10,
        21: 11,
        22: 12,
        23: 13,
        24: 14,
        25: 15,
        26: 16,
        27: 17,
        28: 18,
        29: 19,
        30: 20,
        31: 21
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
        return range(0, self.PORTS_IN_BLOCK + 1)

    @property
    def port_to_eeprom_mapping(self):
        return self._port_to_eeprom_mapping

    def __init__(self):
        eeprom_path = "/sys/class/i2c-adapter/i2c-{0}/{0}-0050/eeprom"

        for x in range(0, self.port_end + 1):
            port_eeprom_path = eeprom_path.format(self.port_to_i2c_mapping[x])
            self.port_to_eeprom_mapping[x] = port_eeprom_path
        SfpUtilBase.__init__(self)

    def get_presence(self, port_num):
        # Check for invalid port_num
        if port_num < self.port_start or port_num > self.port_end:
            return False

        try:
            reg_file = open("/sys/class/swps/port"+str(port_num)+"/present")
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

        global monitor
        port_dict = {}
        with SWPSEventMonitor() as monitor:
            for event in monitor:
                if event['SUBSYSTEM'] == 'swps':
                    #print('SWPS event. From %s, ACTION %s, IF_TYPE %s, IF_LANE %s' % (event['DEVPATH'], event['ACTION'], event['IF_TYPE'], event['IF_LANE']))
                    portname = event['DEVPATH'].split("/")[-1]
                    rc = re.match(r"port(?P<num>\d+)",portname)
                    if rc is not None:
                        if event['ACTION'] == "remove":
                            port_dict[rc.group("num")] = "0"
                        if event['ACTION'] == "add":
                            port_dict[rc.group("num")] = "1"
                        return True, port_dict
                    return False, {}

    def _get_port_eeprom_path(self, port_num, devid):
        sysfs_i2c_adapter_base_path = "/sys/class/i2c-adapter"

        if port_num in self.port_to_eeprom_mapping.keys():
            if devid == self.DOM_EEPROM_ADDR :
                sysfs_sfp_i2c_client_eeprom_path = self.port_to_eeprom_mapping[port_num].replace("0050","0051")
            else:
                sysfs_sfp_i2c_client_eeprom_path = self.port_to_eeprom_mapping[port_num]
        else:
            sysfs_i2c_adapter_base_path = "/sys/class/i2c-adapter"

            i2c_adapter_id = self._get_port_i2c_adapter_id(port_num)
            if i2c_adapter_id is None:
                print("Error getting i2c bus num")
                return None

            # Get i2c virtual bus path for the sfp
            sysfs_sfp_i2c_adapter_path = "%s/i2c-%s" % (sysfs_i2c_adapter_base_path,
                                                        str(i2c_adapter_id))

            # If i2c bus for port does not exist
            if not os.path.exists(sysfs_sfp_i2c_adapter_path):
                print("Could not find i2c bus %s. Driver not loaded?" % sysfs_sfp_i2c_adapter_path)
                return None

            sysfs_sfp_i2c_client_path = "%s/%s-00%s" % (sysfs_sfp_i2c_adapter_path,
                                                        str(i2c_adapter_id),
                                                        hex(devid)[-2:])

            # If sfp device is not present on bus, Add it
            if not os.path.exists(sysfs_sfp_i2c_client_path):
                ret = self._add_new_sfp_device(
                        sysfs_sfp_i2c_adapter_path, devid)
                if ret != 0:
                    print("Error adding sfp device")
                    return None

            sysfs_sfp_i2c_client_eeprom_path = "%s/eeprom" % sysfs_sfp_i2c_client_path

        return sysfs_sfp_i2c_client_eeprom_path

    def get_eeprom_dict(self, port_num):
        """Returns dictionary of interface and dom data.
        format: {<port_num> : {'interface': {'version' : '1.0', 'data' : {...}},
                               'dom' : {'version' : '1.0', 'data' : {...}}}}
        """

        sfp_data = {}

        eeprom_ifraw = self.get_eeprom_raw(port_num)
        eeprom_domraw = self.get_eeprom_dom_raw(port_num)

        if eeprom_ifraw is None:
            return None

        if port_num in self.qsfp_ports:
            sfpi_obj = sff8436InterfaceId(eeprom_ifraw)
            if sfpi_obj is not None:
                sfp_data['interface'] = sfpi_obj.get_data_pretty()
            # For Qsfp's the dom data is part of eeprom_if_raw
            # The first 128 bytes

            sfpd_obj = sff8436Dom(eeprom_ifraw)
            if sfpd_obj is not None:
                sfp_data['dom'] = sfpd_obj.get_data_pretty()

            if eeprom_domraw is not None:
                sfpt_obj = sff8436DomThreshold(eeprom_domraw)
                if sfpt_obj is not None:
                    sfp_data['dom']["data"].update(sfpt_obj.get_data_pretty()["data"])

            return sfp_data

        sfpi_obj = sff8472InterfaceId(eeprom_ifraw)
        if sfpi_obj is not None:
            sfp_data['interface'] = sfpi_obj.get_data_pretty()
            cal_type = sfpi_obj.get_calibration_type()

        if eeprom_domraw is not None:
            sfpd_obj = sff8472Dom(eeprom_domraw, cal_type)
            if sfpd_obj is not None:
                sfp_data['dom'] = sfpd_obj.get_data_pretty()

        return sfp_data
