#!/usr/bin/env python
#
# Copyright (C) 2017 Inventec, Inc.
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

options:
    -h | --help     : this help message
    -d | --debug    : run with debug mode
    -f | --force    : ignore error during installation or clean
command:
    install     : install drivers and generate related sysfs nodes
    clean       : uninstall drivers and remove related sysfs nodes
"""

import os
import commands
import sys, getopt
import logging
import re
import time
from collections import namedtuple

DEBUG = False
args = []
FORCE = 0
i2c_prefix = '/sys/bus/i2c/devices/'


if DEBUG == True:
    print sys.argv[0]
    print 'ARGV      :', sys.argv[1:]


def main():
    global DEBUG
    global args
    global FORCE

    if len(sys.argv)<2:
        show_help()

    options, args = getopt.getopt(sys.argv[1:], 'hdf', ['help',
                                                       'debug',
                                                       'force',
                                                          ])
    if DEBUG == True:
        print options
        print args
        print len(sys.argv)

    for opt, arg in options:
        if opt in ('-h', '--help'):
            show_help()
        elif opt in ('-d', '--debug'):
            DEBUG = True
            logging.basicConfig(level=logging.INFO)
        elif opt in ('-f', '--force'):
            FORCE = 1
        else:
            logging.info('no option')
    for arg in args:
        if arg == 'install':
           install()
        elif arg == 'clean':
           uninstall()
        else:
            show_help()

    return 0

def show_help():
    print __doc__ % {'scriptName' : sys.argv[0].split("/")[-1]}
    sys.exit(0)

def show_log(txt):
    if DEBUG == True:
        print "[D7264]"+txt
    return

def exec_cmd(cmd, show):
    logging.info('Run :'+cmd)
    status, output = commands.getstatusoutput(cmd)
    show_log (cmd +"with result:" + str(status))
    show_log ("     output:"+output)
    if status:
        logging.info('Failed :'+cmd)
        if show:
            print('Failed :'+cmd)
    return  status, output

instantiate =[
'echo inv_eeprom 0x53> /sys/bus/i2c/devices/i2c-0/i2c-2/new_device']

drivers =[
    'gpio_ich',
    'lpc_ich',
    'i2c-i801',
    'i2c-mux',
    'i2c-mux-pca954x',
    'i2c-mux-pca9541',
    'i2c-dev',
    'inv_eeprom',
    'inv_psoc',
    'inv_platform',
#    'inv_cpld',        # triggered by inv_platform
#    'inv_psu',         # triggered by inv_platform
#    'inv_tmp75',       # triggered by inv_platform
#    'inv_ucd90160',    # triggered by inv_platform
    'inv_pthread',
    'swps'
]

# modified kernel driver by inventec
kernal_drivers = [
    'lpc_ich',
    'i2c-mux-pca954x',
    'i2c-mux-pca9541'
]


def system_install():
    global FORCE
    inv_module_path = "/lib/modules/3.16.0-5-amd64/extra"

    #remove default drivers to avoid modprobe order conflicts
    status, output = exec_cmd("rmmod i2c_ismt ", 1)
    status, output = exec_cmd("rmmod i2c-i801 ", 1)
    status, output = exec_cmd("rmmod gpio_ich ", 1)
    status, output = exec_cmd("rmmod lpc_ich ", 1)

    #install drivers
    for i in range(0,len(drivers)):
        if drivers[i] == "gpio_ich":
            status, output = exec_cmd("insmod {}/{}.ko gpiobase=0".format(inv_module_path, drivers[i]), 1)
        elif drivers[i] in kernal_drivers:
            status, output = exec_cmd("insmod {}/{}.ko".format(inv_module_path, drivers[i]), 1)
        else:
            status, output = exec_cmd("modprobe "+drivers[i], 1)

    if status:
	   print output
	   if FORCE == 0:
	      return status

    #instantiate devices
    for i in range(0,len(instantiate)):
       time.sleep(1)
       status, output = exec_cmd(instantiate[i], 1)
    if status:
	   print output
	   if FORCE == 0:
	      return status

    for i in range(100,108):
        status, output =exec_cmd("echo sff8436 0x50 > /sys/bus/i2c/devices/i2c-0/i2c-5/i2c-7/i2c-"+str(i)+"/new_device", 1)
        if status:
            print output
            if FORCE == 0:
                return status
    for i in range(108,116):
        status, output =exec_cmd("echo sff8436 0x50 > /sys/bus/i2c/devices/i2c-0/i2c-5/i2c-8/i2c-"+str(i)+"/new_device", 1)
        if status:
            print output
            if FORCE == 0:
                return status
    for i in range(116,124):
        status, output =exec_cmd("echo sff8436 0x50 > /sys/bus/i2c/devices/i2c-0/i2c-5/i2c-9/i2c-"+str(i)+"/new_device", 1)
        if status:
            print output
            if FORCE == 0:
                return status
    for i in range(124,132):
        status, output =exec_cmd("echo sff8436 0x50 > /sys/bus/i2c/devices/i2c-0/i2c-5/i2c-10/i2c-"+str(i)+"/new_device", 1)
        if status:
            print output
            if FORCE == 0:
                return status
    for i in range(132,140):
        status, output =exec_cmd("echo sff8436 0x50 > /sys/bus/i2c/devices/i2c-0/i2c-5/i2c-11/i2c-"+str(i)+"/new_device", 1)
        if status:
            print output
            if FORCE == 0:
                return status
    for i in range(140,148):
        status, output =exec_cmd("echo sff8436 0x50 > /sys/bus/i2c/devices/i2c-0/i2c-5/i2c-12/i2c-"+str(i)+"/new_device", 1)
        if status:
            print output
            if FORCE == 0:
                return status
    for i in range(148,156):
        status, output =exec_cmd("echo sff8436 0x50 > /sys/bus/i2c/devices/i2c-0/i2c-5/i2c-13/i2c-"+str(i)+"/new_device", 1)
        if status:
            print output
            if FORCE == 0:
                return status
    for i in range(156,164):
        status, output =exec_cmd("echo sff8436 0x50 > /sys/bus/i2c/devices/i2c-0/i2c-5/i2c-14/i2c-"+str(i)+"/new_device", 1)
        if status:
            print output
            if FORCE == 0:
                return status
    return


def system_ready():
    if not device_found():
        return False
    return True

def install():
    if not device_found():
        print "No device, installing...."
        status = system_install()
        if status:
            if FORCE == 0:
                return  status
    else:
        print " D7264 devices detected...."
    return

def uninstall():
    global FORCE
    #uninstall drivers
    for i in range(len(drivers)-1,-1,-1):
       status, output = exec_cmd("rmmod "+drivers[i], 1)
    status, output = exec_cmd("rmmod gpio_ich ", 1)
    if status:
	   print output
	   if FORCE == 0:
	      return status
    return

def device_found():
    ret1, log = exec_cmd("ls "+i2c_prefix+"*0072", 0)
    ret2, log = exec_cmd("ls "+i2c_prefix+"i2c-2", 0)
    return not(ret1 or ret2)

if __name__ == "__main__":
    main()
