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
    PORT_END = 55
    PORTS_IN_BLOCK = 56
    QSFP_PORT_START = 48
    QSFP_PORT_END = 55

    SWPS_FOLDER = "/sys/class/swps/"

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
            reg_file = open("/sys/class/swps/port"+str(port_num+1)+"/lpmod")
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
            reg_file = open("/sys/class/swps/port"+str(port_num+1)+"/lpmod", "r+")
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
        QSFP_RESET_REGISTER_DEVICE_FILE = "/sys/class/swps/port"+str(port_num+1)+"/reset"
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
        #"""
        #TODO: This function need to be implemented
        #"""
        #raise NotImplementedError
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
                            remove_num = int(rc.group("num")) -1
                            port_dict[remove_num] = "0"
                            #port_dict[rc.group("num")] = "0"
                        if event['ACTION'] == "add":
                            add_num = int(rc.group("num")) -1
                            port_dict[add_num] = "1"
                            #port_dict[rc.group("num")] = "1"
                        return True, port_dict
                    return False, {}

    def get_transceiver_dom_info_dict(self, port_num):
        import re
        dom_info_dict = {}
        # initial all entires
        dom_info_dict['temperature'] = "N/A"
        dom_info_dict['voltage'] = "N/A"
        dom_info_dict['rx1power'] = "-inf"
        dom_info_dict['rx2power'] = "-inf"
        dom_info_dict['rx3power'] = "-inf"
        dom_info_dict['rx4power'] = "-inf"
        dom_info_dict['tx1bias'] = "N/A"
        dom_info_dict['tx2bias'] = "N/A"
        dom_info_dict['tx3bias'] = "N/A"
        dom_info_dict['tx4bias'] = "N/A"
        dom_info_dict['tx1power'] = "-inf"
        dom_info_dict['tx2power'] = "-inf"
        dom_info_dict['tx3power'] = "-inf"
        dom_info_dict['tx4power'] = "-inf"
        dom_info_dict['wavelength'] = "N/A"
        dom_info_dict['rx_am'] = "N/A"
    
        file_list = os.listdir(self.SWPS_FOLDER)
        portname = "port{0}".format(port_num + 1)
        if portname in os.listdir(self.SWPS_FOLDER):
            path = "{0}{1}".format(self.SWPS_FOLDER,portname)
    
            # temperature
            try:
                with open( "{0}/temperature".format(path), 'rb') as readPtr:
                    dom_info_dict['temperature'] = readPtr.read().replace('\n', '')
            except:
                pass
    
            # voltage
            try:
                with open( "{0}/voltage".format(path), 'rb') as readPtr:
                    dom_info_dict['voltage'] = readPtr.read().replace('\n', '')
            except:
                pass
    
            # rx_power
            try:
                with open( "{0}/rx_power".format(path), 'rb') as readPtr:
                    count = 1
                    for line in readPtr:
                        power = re.search(r"(RX\-[1234]\:)*(?P<rx>\d+\.\d+)", line)
                        if power is not None:
                            dom_info_dict['rx{0}power'.format(count)] = power.group("rx")
                            count = count + 1
            except:
                pass
    
            # tx_bias
            try:
                with open( "{0}/tx_bias".format(path), 'rb') as readPtr:
                    count = 1
                    for line in readPtr:
                        bias = re.search(r"(TX\-[1234]\:)*(?P<bias>\d+\.\d+)", line)
                        if bias is not None:
                            dom_info_dict['tx{0}bias'.format(count)] = bias.group("bias")
                            count = count + 1
            except:
                pass
    
            # tx_power
            try:
                with open( "{0}/tx_power".format(path), 'rb') as readPtr:
                    count = 1
                    for line in readPtr:
                        power = re.search(r"(TX\-[1234]\:)*(?P<tx>\d+\.\d+)", line)
                        if power is not None:
                            dom_info_dict['tx{0}power'.format(count)] = power.group("tx")
                            count = count + 1
            except:
                pass
    
        return dom_info_dict
    
