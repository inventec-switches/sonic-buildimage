#!/usr/bin/env python
#
# Copyright (C) 2018 Inventec, Inc.
# 
# Editor: James Huang ( Huang.James@inventec.com )
#  
"""
Usage: %(scriptName)s [options] command object

Auto detecting the transceiver and set the correct if_type value

options:
    -h | --help     : this help message
    -d | --debug    : run with debug mode
   
"""

try:
    import os
    import commands
    import sys, getopt
    import logging
    import re
    import time
    import datetime
    import syslog
    import os
    import socket
    from collections import OrderedDict    
    from sonic_sfp.bcmshell import bcmshell
    
except ImportError as e:
    raise ImportError("%s - required module not found" % str(e))

DEBUG = False
args = []
INV_REDWOOD_PLATFORM = "SONiC-Inventec-d7032-100"
INV_CYPRESS_PLATFORM = "SONiC-Inventec-d7054"
INV_SEQUOIA_PLATFORM = "SONiC-Inventec-d7264"
INV_MAPLE_PLATFORM = "SONiC-Inventec-d6556"
INV_MAGNOLIA_PLATFORM = "SONiC-Inventec-d6254qs"

NETLINK_KOBJECT_UEVENT = 15

SWPS_PATH = "/sys/class/swps"


def log_message( level, string ):
    syslog.openlog("transceiver_monitor", syslog.LOG_PID, facility=syslog.LOG_DAEMON)
    syslog.syslog( level, string )

class PortUtil(bcmshell):

    port_to_bcm_mapping = dict()         
    eagle_list = []
    platform = None
    
    # === Initial process == #
    def initial(self):
        self.initial_port_to_bcm_mapping()
        self.initial_sal_config_list()
        self.parsing_port_list()
    
    
    # === Platform === #
    def get_platform(self):
        if self.platform is None:
            self.platform = os.popen("uname -n").read().strip()
        return self.platform
    
    # === Eagle Port === #   
    def get_eagle_port(self):
        if len(self.eagle_list) == 0:
            self.parsing_eagle_port()
        return self.eagle_list
        
    def parsing_eagle_port(self):
        name = self.get_platform()
        if name is not None:
            if name == INV_REDWOOD_PLATFORM:
                self.eagle_list = [66,100]
            elif name == INV_CYPRESS_PLATFORM:
                self.eagle_list = [66,100]
            elif name == INV_SEQUOIA_PLATFORM:
                self.eagle_list = [66,100]
            elif name == INV_MAPLE_PLATFORM:
                self.eagle_list = [66,130]
            else:
                self.eagle_list = []
    
    # === Port map === # 
    # SWPS_port --
    #            | - BCM port id
    #            | - BCM port name
    #            | - Lane <<
    #            | - Speed
    
    def initial_port_to_bcm_mapping(self):
        for index in os.listdir(SWPS_PATH):
            portObject = re.search(r"(?P<port_name>port\d+)",index)
            if portObject is not None:   
               self.port_to_bcm_mapping[portObject.group("port_name")] = {"bcm_id":None, "bcm_name":None, "lane":None, "speed": None}
               with open( "{0}/{1}/if_lane".format(SWPS_PATH, index), 'rb') as readPtr:
                    content = readPtr.read().strip()
                    self.port_to_bcm_mapping[portObject.group("port_name")]["lane"] = map(int, content.split(","))
                    
    def get_port_to_bcm_mapping(self):  
        if self.port_to_bcm_mapping is None:
            return dict()
        else:
            return self.port_to_bcm_mapping     
    
    def show_port_to_bcm_mapping(self): 
        for key,value in self.port_to_bcm_mapping.iteritems():
            print "{0}---{1}".format(key, value)    

    # === config === # 
    # SWPS_port --
    #            | - BCM port id <<
    #            | - BCM port name
    #            | - Lane
    #            | - Speed   
    #    
    def initial_sal_config_list( self ):
        content = self.run("config")  
        for line in content.split("\n"):
            ConfigObject = re.search(r"portmap\_(?P<bcm_id>\d+)\=(?P<lane_id>\d+)\:\d+",line)
            if ConfigObject is not None:   
                if int(ConfigObject.group("bcm_id")) not in self.get_eagle_port():
                    for key,value in self.port_to_bcm_mapping.iteritems():
                        if int(ConfigObject.group("lane_id")) in value["lane"]:
                            value["bcm_id"] = int(ConfigObject.group("bcm_id"))
                            break
                    
    # === ps === # 
    # SWPS_port --
    #            | - BCM port id 
    #            | - BCM port name <<
    #            | - Lane
    #            | - Speed   <<
    #    
    def parsing_port_list(self):
        content = self.run("ps")
        count = 0
        for line in content.split("\n"):
            PSObject = re.search(r"(?P<port_name>(xe|ce)\d+)\(\s*(?P<bcm_id>\d+)\).+\s+(?P<speed>\d+)G",line)
            if PSObject is not None:
                if int(PSObject.group("bcm_id")) not in self.get_eagle_port():                    
                    for key,value in self.port_to_bcm_mapping.iteritems():
                        if int(PSObject.group("bcm_id")) == value["bcm_id"]:
                            value["bcm_name"] = PSObject.group("port_name")
                            value["speed"] = int(PSObject.group("speed"))*1000
                            break
                
    
    def execute_command(self, cmd):
        return self.run(cmd)
        
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
        
def main():

    global port_obj
    global monitor
    port_obj = PortUtil()
    port_obj.initial()
    port_obj.show_port_to_bcm_mapping()
    
    os.system("echo inventec > /sys/class/swps/module/reset_swps")
    
    with SWPSEventMonitor() as monitor:
        for event in monitor:
            if event['SUBSYSTEM'] == 'swps':
                #print('SWPS event. From %s, ACTION %s, IF_TYPE %s, IF_LANE %s' % (event['DEVPATH'], event['ACTION'], event['IF_TYPE'], event['IF_LANE']))
                portname = event['DEVPATH'].split("/")[-1]
                try:
                    if port_obj.get_platform() == INV_SEQUOIA_PLATFORM or port_obj.get_platform() == INV_MAPLE_PLATFORM :
                        port_obj.parsing_port_list()  
                        port_obj.execute_command( "port {0} if={1} speed={2}".format( port_obj.port_to_bcm_mapping[portname]["bcm_name"], event['IF_TYPE'], port_obj.port_to_bcm_mapping[portname]["speed"] ) )
                    else:
                        port_obj.execute_command( "port {0} if={1}".format( port_obj.port_to_bcm_mapping[portname]["bcm_name"], event['IF_TYPE'] ) )
                except Exception, e:
                        log_message( syslog.LOG_WARNING, "Exception. The warning is {0}".format(str(e)) )
                        raise Exception("[swps_monitor.py] Exception. The warning is {0}".format(str(e)) )
                
if __name__ == "__main__":
    main()
    



