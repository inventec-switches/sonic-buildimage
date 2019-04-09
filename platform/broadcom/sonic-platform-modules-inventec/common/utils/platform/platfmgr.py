#!/usr/bin/python

#
# Copyright (C) 2018 Inventec, Inc.
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


import sys
import os
import re
import syslog
import time
from sonic_sfp.bcmshell import bcmshell
from swsssdk import SonicV2Connector
from msgLog import msg_to_syslog
from invCommon import InvPlatform



# sleep time check(second)
REDISDBTIMECHECK        = 1
SWSSTIMECHECK           = 3
BCMSHELLTIMECHECK       = 5

# define board type
INV_MAGNOLIA            = "x86_64-inventec_d6254qs-r0"
INV_REDWOOD             = "x86_64-inventec_d7032q28b-r0"
INV_CYPRESS             = "x86_64-inventec_d7054q28b-r0"
INV_MAPLE               = "x86_64-inventec_d6356-r0"
INV_MAPLE_EVT1          = "x86_64-inventec_d6556-r0"
INV_SEQUOIA             = "x86_64-inventec_d7264q28b-r0"

#define daemon
PLATFROM                = None
BASE_PATH               = None
PID_PATH                = '/var/run'
LED_DAEMON              = None
LED_PIDFILE             = None
SWPS_DAEMON             = None
SWPS_PIDFILE            = None
ASIC_DAEMON             = None
ASIC_PIDFILE            = None


RDB = None
def rdb_connect():
    RDB.connect(RDB.ASIC_DB)


def rdb_get_all(database, key):
    """
    parameter:
        database    -- database name in sonic redis-db
        key         -- specific key in database
    return:
        entry       -- the content in the key
    """
    entry = RDB.get_all(database, key, blocking=True)
    return entry



def platform_path_init():
    global PLATFROM
    global BASE_PATH
    global LED_DAEMON
    global LED_PIDFILE
    global SWPS_DAEMON
    global SWPS_PIDFILE
    global ASIC_DAEMON
    global ASIC_PIDFILE

    invP = InvPlatform()
    while True:
        platformName = invP.get_platform()
        if platformName is not "None":
            break

    if platformName.rstrip() == INV_MAGNOLIA:
        msg_to_syslog('info', "found platform: {0}".format(INV_MAGNOLIA))
        PLATFROM        = INV_MAGNOLIA
        BASE_PATH       = '/usr/share/sonic/device/{0}/plugins'.format(PLATFROM)
        ASIC_DAEMON     = '{0}/asic_monitor.py'.format(BASE_PATH)
        ASIC_PIDFILE    = '{0}/asic_monitor'.format(PID_PATH)
        LED_DAEMON      = '{0}/led_proc.py'.format(BASE_PATH)
        LED_PIDFILE     = '{0}/led_proc'.format(PID_PATH)
        SWPS_DAEMON     = '{0}/transceiver_monitor.py'.format(BASE_PATH)
        SWPS_PIDFILE    = '{0}/transceiver_monitor'.format(PID_PATH)
    elif platformName.rstrip() == INV_REDWOOD:
        msg_to_syslog('info', "found platform: {0}".format(INV_REDWOOD))
        PLATFROM        = INV_REDWOOD
        BASE_PATH       = '/usr/share/sonic/device/{0}/plugins'.format(PLATFROM)
        ASIC_DAEMON     = '{0}/asic_monitor.py'.format(BASE_PATH)
        ASIC_PIDFILE    = '{0}/asic_monitor'.format(PID_PATH)
        LED_DAEMON      = '{0}/led_proc.py'.format(BASE_PATH)
        LED_PIDFILE     = '{0}/led_proc'.format(PID_PATH)
        SWPS_DAEMON     = '{0}/swps_monitor.py'.format(BASE_PATH)
        SWPS_PIDFILE    = '{0}/swps_monitor'.format(PID_PATH)
    elif platformName.rstrip() == INV_CYPRESS:
        msg_to_syslog('info', "found platform: {0}".format(INV_CYPRESS))
        PLATFROM        = INV_CYPRESS
        BASE_PATH       = '/usr/share/sonic/device/{0}/plugins'.format(PLATFROM)
        ASIC_DAEMON     = '{0}/asic_monitor.py'.format(BASE_PATH)
        ASIC_PIDFILE    = '{0}/asic_monitor'.format(PID_PATH)
        LED_DAEMON      = '{0}/led_proc.py'.format(BASE_PATH)
        LED_PIDFILE     = '{0}/led_proc'.format(PID_PATH)
        SWPS_DAEMON     = '{0}/swps_monitor.py'.format(BASE_PATH)
        SWPS_PIDFILE    = '{0}/swps_monitor'.format(PID_PATH)
    elif platformName.rstrip() == INV_SEQUOIA:
        msg_to_syslog('info', "found platform: {0}".format(INV_SEQUOIA))
        PLATFROM        = INV_SEQUOIA
        BASE_PATH       = '/usr/share/sonic/device/{0}/plugins'.format(PLATFROM)
        ASIC_DAEMON     = '{0}/asic_monitor.py'.format(BASE_PATH)
        ASIC_PIDFILE    = '{0}/asic_monitor'.format(PID_PATH)
        LED_DAEMON      = '{0}/led_proc.py'.format(BASE_PATH)
        LED_PIDFILE     = '{0}/led_proc'.format(PID_PATH)
        SWPS_DAEMON     = '{0}/transceiver_monitor.py'.format(BASE_PATH)
        SWPS_PIDFILE    = '{0}/transceiver_monitor'.format(PID_PATH)
    elif platformName.rstrip() == INV_MAPLE:
        msg_to_syslog('info', "found platform: {0}".format(INV_MAPLE))
        PLATFROM        = INV_MAPLE
        BASE_PATH       = '/usr/share/sonic/device/{0}/plugins'.format(PLATFROM)
        ASIC_DAEMON     = '{0}/asic_monitor.py'.format(BASE_PATH)
        ASIC_PIDFILE    = '{0}/asic_monitor'.format(PID_PATH)
        LED_DAEMON      = '{0}/led_proc.py'.format(BASE_PATH)
        LED_PIDFILE     = '{0}/led_proc'.format(PID_PATH)
        SWPS_DAEMON     = '{0}/swps_monitor.py'.format(BASE_PATH)
        SWPS_PIDFILE    = '{0}/swps_monitor'.format(PID_PATH)
    elif platformName.rstrip() == INV_MAPLE_EVT1:
        msg_to_syslog('info', "found platform: {0}".format(INV_MAPLE_EVT1))
        PLATFROM        = INV_MAPLE_EVT1
        BASE_PATH       = '/usr/share/sonic/device/{0}/plugins'.format(PLATFROM)
        ASIC_DAEMON     = '{0}/asic_monitor.py'.format(BASE_PATH)
        ASIC_PIDFILE    = '{0}/asic_monitor'.format(PID_PATH)
        LED_DAEMON      = '{0}/led_proc.py'.format(BASE_PATH)
        LED_PIDFILE     = '{0}/led_proc'.format(PID_PATH)
        SWPS_DAEMON     = '{0}/swps_monitor.py'.format(BASE_PATH)
        SWPS_PIDFILE    = '{0}/swps_monitor'.format(PID_PATH)
    else:
        msg_to_syslog('warning', "platform not found")



def wake_process():
    start = '/sbin/start-stop-daemon --quiet --oknodo --pidfile {0} --make-pidfile --startas {1} --start --background -- $DAEMON_OPTS'

    # daemon list ### xxx = start.format(xxx_PIDFILE, xxx_DAEMON)
    asic_monitor    = start.format(ASIC_PIDFILE, ASIC_DAEMON)
    led_proc        = start.format(LED_PIDFILE, LED_DAEMON)


    # add to the start command list [aaa, bbb, ccc]
    cmdList = [asic_monitor, led_proc]
    for cmd in cmdList:
        os.system(cmd)
        msg_to_syslog('debug', cmd)

    msg_to_syslog('info', 'wake all invSyncd sub-daemon')



def kill_process():
    stop = '/sbin/start-stop-daemon --quiet --oknodo --stop --pidfile {0} --retry 10'

    # daemon list ### xxx = stop.format(xxx_PIDFILE)
    asic_monitor    = stop.format(ASIC_PIDFILE)
    led_proc        = stop.format(LED_PIDFILE)
    
    # add to the stop command list [xxx, yyy, zzz]
    cmdList = [asic_monitor, led_proc]
    for cmd in cmdList:
        os.system(cmd)

    msg_to_syslog('info', 'kill all platfmgr sub-daemon')



def sync_bcmsh_socket():
    waitSyncd   = True
    retryCount  = 0

    # retry new a bcmshell object
    try:
        shell = bcmshell()
    except Exception, e:
        msg_to_syslog('debug', "{0}".format(str(e)))
        retryCount += 1

    # retry the socket connection for Echo
    while True:
        try:
            time.sleep(BCMSHELLTIMECHECK)
            rv = shell.run("Echo")
            msg_to_syslog('debug', 'bcmcmd: {0}'.format(rv))
            if rv.strip() == "Echo":
                break
        except Exception, e:
            msg_to_syslog('debug', "{0}, Retry times({1})".format(str(e),retryCount))
            retryCount += 1

    msg_to_syslog('info', "bcmshell socket create successfully")



def check_swss_service():
    """
    return:
        False   -- inactive
        True    -- active
    """

    status  = None
    cmd     = "service swss status"
    nLine   = 0

    # check swss service status
    for line in os.popen(cmd).read().split("\n"):
        if nLine == 2:
            reObj = re.search(r"Active\:.+\((?P<status>\w+)\)", line)
            if reObj is not None:
                status = reObj.group("status")
        elif nLine > 2:
            break
        nLine += 1

    if status == "running":
        # check swss container and syncd container is ready
        cmd = 'docker exec swss echo -ne "SELECT 1\\nHLEN HIDDEN" | redis-cli | sed -n 2p'
        while True:
            try:
                rv = os.popen(cmd).read().rstrip()
                if rv.isdigit() and rv in ["3","4","5"] :
                    content = rdb_get_all(RDB.ASIC_DB, "HIDDEN")
                    if len(content) >= 3:
                        break
            except Exception as e:
                msg_to_syslog('debug', str(e))
        return True
    else:
        return False



def main():
    syslog.openlog("platfmgr", syslog.LOG_PID, facility=syslog.LOG_DAEMON)

    # check redis Database is ready
    global RDB
    cmd = 'redis-cli ping | grep -c PONG'
    while True:
        try:
            rv = os.popen(cmd).read()
            if int(rv.rstrip()) > 0:
                RDB = SonicV2Connector(host="127.0.0.1")
                rdb_connect()
                msg_to_syslog('debug', 'redis database........ready')
                break
        except Exception as e:
            msg_to_syslog('debug', str(e))
        time.sleep(REDISDBTIMECHECK)

    # init plarform path
    platform_path_init()

    # main thread for checing swss status
    thread  = True
    wake    = False
    while thread:
        readyGo = check_swss_service()

        while readyGo:
            msg_to_syslog('debug', 'swss active')
            # wake process one time
            if not wake:
                sync_bcmsh_socket()
                wake_process()
                wake = True
            time.sleep(SWSSTIMECHECK)
            readyGo = check_swss_service()

        while not readyGo:
            msg_to_syslog('debug', 'swss inactive')
            # kill process one time
            if wake:
                kill_process()
                wake = False
            time.sleep(SWSSTIMECHECK)
            readyGo = check_swss_service()


    syslog.closelog()


if __name__ == "__main__":
    main()

