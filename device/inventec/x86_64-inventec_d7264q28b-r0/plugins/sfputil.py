# sfputil.py
#
# Platform-specific SFP transceiver interface for SONiC
#

try:
    import time
    import socket, re,os
    from collections import OrderedDict
    from sonic_sfp.sfputilbase import SfpUtilBase
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
    PORT_END = 63
    PORTS_IN_BLOCK = 64
    QSFP_PORT_START = 0
    QSFP_PORT_END = 63

    _port_to_eeprom_mapping = {}
    port_to_i2c_mapping = {
        0: 10,
        1: 11,
        2: 12,
        3: 13,
        4: 14,
        5: 15,
        6: 16,
        7: 17,
        8: 18,
        9: 19,
        10: 20,
        11: 21,
        12: 22,
        13: 23,
        14: 24,
        15: 25,
        16: 26,
        17: 27,
        18: 28,
        19: 29,
        20: 30,
        21: 31,
        22: 32,
        23: 33,
        24: 34,
        25: 35,
        26: 36,
        27: 37,
        28: 38,
        29: 39,
        30: 40,
        31: 41,
        32: 45,
        33: 44,
        34: 43,
        35: 42,
        36: 49,
        37: 48,
        38: 47,
        39: 46,
        40: 53,
        41: 52,
        42: 51,
        43: 50,
        44: 57,
        45: 56,
        46: 55,
        47: 54,
        48: 61,
        49: 60,
        50: 59,
        51: 58,
        52: 65,
        53: 64,
        54: 63,
	55: 62,
	56: 69,
	57: 68,
	58: 67,
	59: 66,
	60: 73,
	61: 72,
	62: 71,
	63: 70
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
        eeprom_path = "/sys/class/swps/port{0}/eeprom"
        sfp_a2_eeprom_path = "/sys/class/swps/port{0}/A2"
        qsfp_p3_eeprom_path = "/sys/class/swps/port{0}/Page03"

        for x in range(0, self.port_end + 1):
            port_eeprom_path = eeprom_path.format(x)
            self.port_to_eeprom_mapping[x] = port_eeprom_path

        # Set the SFP A2h i2c eeprom path
        for y in range(0, self.qsfp_port_start):
            if self.port_to_eeprom_mapping.has_key(y) :
                self.port_to_eeprom_mapping[y] = [ self.port_to_eeprom_mapping[y] , sfp_a2_eeprom_path.format(y)]

        # Set the QSFP Page03 i2c eeprom path
        for z in range(self.qsfp_port_start, self.qsfp_port_end +1 ):
            if self.port_to_eeprom_mapping.has_key(z) :
                self.port_to_eeprom_mapping[z] = [ self.port_to_eeprom_mapping[z] , qsfp_p3_eeprom_path.format(z)]

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

