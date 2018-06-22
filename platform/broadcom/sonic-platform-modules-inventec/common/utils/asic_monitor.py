#!/usr/bin/env python
#
# Copyright (C) 2018 Inventec, Inc.
# 
# Editor: James Huang ( Huang.James@inventec.com )
#  
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
    import datetime
    import syslog
    from sonic_sfp.bcmshell import bcmshell
    
except ImportError as e:
    raise ImportError("%s - required module not found" % str(e))

DEBUG = False
args = []
INV_REDWOOD_PLATFORM = "SONiC-Inventec-d7032-100"
INV_CYPRESS_PLATFORM = "SONiC-Inventec-d7054"
INV_SEQUOIA_PLATFORM = "SONiC-Inventec-d7264"
PSOC_NAME = "name"
HWMON_PATH = "/sys/class/hwmon/"
SWITCH_TEMP_FILE_NAME = "switch_tmp"

def show_help():
    print __doc__ % {'scriptName' : sys.argv[0].split("/")[-1]}
    sys.exit(0)

def log_message( string ):
    print string
    syslog.openlog("asic_monitor", syslog.LOG_PID, facility=syslog.LOG_DAEMON)
    syslog.syslog(syslog.LOG_NOTICE, string)

class BCMUtil(bcmshell):

    asic_temperature = 0
    platform = None
    
    def get_platform(self):
        if self.platform is None:
            self.platform = os.popen("uname -n").read().strip()
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
        self.cmd(cmd)

def main():
    try:
        global DEBUG  
        global bcm_obj
        
        initalNotOK = True
        retestCount = 0 
        while initalNotOK :
            try:                
                bcm_obj = BCMUtil()
                initalNotOK = False
            except Exception, e:               
                log_message("Exception. The warning is {0}, Retry again ({1})".format(str(e),retestCount) )                    
                retestCount = retestCount + 1
            time.sleep(5)
         
        log_message( "Object initialed successfully" )  

        options, args = getopt.getopt(sys.argv[1:], 'hd', ['help',
                                                           'debug'
                                                              ])
        for opt, arg in options:
            if opt in ('-h', '--help'):
                show_help()
            elif opt in ('-d', '--debug'):            
                DEBUG = True
                logging.basicConfig(level=logging.INFO)
            else:
                logging.info("no option")
        
        while 1 :
            bcm_obj.parsing_asic_temp()
            for index in os.listdir(HWMON_PATH):
                file_list = os.listdir("{0}/{1}/device/".format(HWMON_PATH,index))
                if PSOC_NAME in file_list :
                    with open( "{0}/{1}/device/{2}".format(HWMON_PATH, index, PSOC_NAME), 'rb') as readPtr:
                        content = readPtr.read().strip()
                        if bcm_obj.get_platform() == INV_SEQUOIA_PLATFORM :
                            if content == "inv_bmc" and SWITCH_TEMP_FILE_NAME in file_list:  
                                os.system("echo {0} > {1}/{2}/device/{3}".format( ( bcm_obj.get_asic_temperature() * 1000 ), HWMON_PATH, index, SWITCH_TEMP_FILE_NAME )) 
                                break
                        else :
                            if content == "inv_psoc" and SWITCH_TEMP_FILE_NAME in file_list:
                                os.system("echo {0} > {1}/{2}/device/{3}".format( ( bcm_obj.get_asic_temperature() * 1000 ), HWMON_PATH, index, SWITCH_TEMP_FILE_NAME )) 
                                break
            time.sleep(5)

    except (Exception, KeyboardInterrupt) as e:
        log_message("Terminating this python daemon ({0})".format(e))   
        syslog.closelog()
        del bcm_obj

if __name__ == "__main__":
    main()

