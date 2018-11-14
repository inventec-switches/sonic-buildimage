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




# sleep time check(second)
REDISDBTIMECHECK        = 1
SWSSTIMECHECK           = 3
BCMSHELLTIMECHECK       = 5

#define daemon
BASE_PATH               = '/usr/local/lib/python2.7/dist-packages/inventec_plugin/inv_syncd'
PID_PATH                = '/var/run'
PORTSTATD_DAEMON        = '{0}/portstat_prepared.py'.format(BASE_PATH)
PORTSTATD_PIDFILE       = '{0}/portstat_prepared'.format(PID_PATH)


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



def wake_process():
    start = '/sbin/start-stop-daemon --quiet --oknodo --pidfile {0} --make-pidfile --startas {1} --start --background -- $DAEMON_OPTS'

    # daemon list ### xxx = start.format(xxx_PIDFILE, xxx_DAEMON)
    portstatPrepared    = start.format(PORTSTATD_PIDFILE, PORTSTATD_DAEMON)

    # add to the start command list [aaa, bbb, ccc]
    cmdList = [portstatPrepared]
    for cmd in cmdList:
        os.system(cmd)
        msg_to_syslog('debug', cmd)

    msg_to_syslog('info', 'wake all invSyncd sub-daemon')



def kill_process():
    stop = '/sbin/start-stop-daemon --quiet --oknodo --stop --pidfile {0} --retry 10'

    # daemon list ### xxx = stop.format(xxx_PIDFILE)
    
    # add to the stop command list [xxx, yyy, zzz]
    cmdList = []
    for cmd in cmdList:
        os.system(cmd)

    msg_to_syslog('info', 'kill all invSyncd sub-daemon')


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
    syslog.openlog("invSyncd", syslog.LOG_PID, facility=syslog.LOG_DAEMON)

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

