#!/usr/bin/env python
#
# Copyright (C) 2018 Inventec, Inc.
# 
# Editor: James Huang ( Huang.James@inventec.com )
#  
"""
Usage: %(scriptName)s [options] command object

Auto detecting the Chipset temperature and update

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
    import syslog
    from invCommon import InvPlatform
    from sonic_sfp.bcmshell import bcmshell
    
except ImportError as e:
    raise ImportError("%s - required module not found" % str(e))

args = []
INV_REDWOOD_PLATFORM  = "x86_64-inventec_d7032q28b-r0"
INV_CYPRESS_PLATFORM  = "x86_64-inventec_d7054q28b-r0"
INV_SEQUOIA_PLATFORM  = "x86_64-inventec_d7264q28b-r0"
INV_MAPLE_PLATFORM    = "x86_64-inventec_d6556-r0"
INV_MAGNOLIA_PLATFORM = "x86_64-inventec_d6254qs-r0"

PSOC_NAME = "name"
HWMON_PATH = "/sys/class/hwmon"
SWITCH_TEMP_FILE_NAME = "switch_tmp"

def log_message( level, string ):
    syslog.openlog("asic_monitor", syslog.LOG_PID, facility=syslog.LOG_DAEMON)
    syslog.syslog( level, string )

class BCMUtil(bcmshell):

    asic_temperature = 0
    platform = None
    
    def get_platform(self):
        if self.platform is None:
            platfromObj = InvPlatform()
            self.platform = platfromObj.get_platform()
        return self.platform
        
    def get_asic_temperature( self ):
        return self.asic_temperature
        
    def set_asic_temperature( self, temp ):
        self.asic_temperature = temp
                
    def parsing_asic_temp(self):
        content = self.run("show temp")
        for line in content.split("\n"):
            TempObject = re.search(r"(average current temperature is)\s+(?P<temperature_high>\d+)\.(?P<temperature_low>\d+)",line)
            if TempObject is not None:
                self.set_asic_temperature( int( TempObject.group("temperature_high") ) )
        
    def execute_command(self, cmd):
        return self.run(cmd)
     
        
def main():

    global bcm_obj
    bcm_obj = BCMUtil()
    log_message( syslog.LOG_INFO, "Object initialed successfully" )

    while 1 :
        try:
            bcm_obj.parsing_asic_temp()
            for index in os.listdir(HWMON_PATH):
                if SWITCH_TEMP_FILE_NAME in os.listdir("{0}/{1}".format(HWMON_PATH,index)) :
                    #print "echo {0} > {1}/{2}/{3}".format( ( bcm_obj.get_asic_temperature() * 1000 ), HWMON_PATH, index, SWITCH_TEMP_FILE_NAME )
                    os.system("echo {0} > {1}/{2}/{3}".format( ( bcm_obj.get_asic_temperature() * 1000 ), HWMON_PATH, index, SWITCH_TEMP_FILE_NAME ))
                    break
                else :
                    file_list = os.listdir("{0}/{1}/device".format(HWMON_PATH,index))
                    if PSOC_NAME in file_list :
                        with open( "{0}/{1}/device/{2}".format(HWMON_PATH, index, PSOC_NAME), 'rb') as readPtr:
                            content = readPtr.read().strip()
                            if bcm_obj.get_platform() == INV_SEQUOIA_PLATFORM :
                                if content == "inv_bmc" and SWITCH_TEMP_FILE_NAME in file_list :
                                    os.system("echo {0} > {1}/{2}/device/{3}".format( ( bcm_obj.get_asic_temperature() * 1000 ), HWMON_PATH, index, SWITCH_TEMP_FILE_NAME ))
                                    break
                            else :
                                if content == "inv_psoc" and SWITCH_TEMP_FILE_NAME in file_list :
                                    #print "echo {0} > {1}/{2}/device/{3}".format( ( bcm_obj.get_asic_temperature() * 1000 ), HWMON_PATH, index, SWITCH_TEMP_FILE_NAME )
                                    os.system("echo {0} > {1}/{2}/device/{3}".format( ( bcm_obj.get_asic_temperature() * 1000 ), HWMON_PATH, index, SWITCH_TEMP_FILE_NAME ))
                                    break
        except Exception, e:
            log_message( syslog.LOG_WARNING, "Exception. The warning is {0}".format(str(e)) )
            raise Exception("[asic_monitor.py] Exception. The warning is {0}".format(str(e)))

        time.sleep(120)

    syslog.closelog()
    del bcm_obj

if __name__ == "__main__":
    main()



