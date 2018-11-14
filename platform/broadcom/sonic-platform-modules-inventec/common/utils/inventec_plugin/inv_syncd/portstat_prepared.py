#!/usr/bin/env python
#
# Copyright (C) 2018 Inventec, Inc.
# 
# Editor: James Huang ( Huang.James@inventec.com )
#  
try:
    import os
    import commands
    import sys, getopt
    import re
    import time
    import syslog
    import os
    from swsssdk import SonicV2Connector
    from inventec_plugin.msgLog import msg_to_syslog
except ImportError as e:
    raise ImportError("%s - required module not found" % str(e))
    
def main():
    
    # config syslog feature 
    syslog.openlog("portstat_prepared", syslog.LOG_PID, facility=syslog.LOG_DAEMON)
    
    notReady = True   
    while notReady:
        try :
            dbConnect=SonicV2Connector( host="127.0.0.1" )
            dbConnect.connect(dbConnect.COUNTERS_DB)
            interface_name_map = dbConnect.get_all(dbConnect.COUNTERS_DB, 'COUNTERS_PORT_NAME_MAP', blocking=False)
            EthernetList = interface_name_map.keys()
            msg_to_syslog("info", "Get Ethernet list({0}) from COUNTERS_DB".format(len(EthernetList)))
            notReady = False
        except Exception, e:
            msg_to_syslog("warning", "Exception. The warning is {0}".format(str(e)))
            time.sleep(5)
            
    uid = str(os.getuid())
    cnstat_dir = "portstat-" + uid
    
    notReady = True
    while notReady:
        file_list = os.listdir("/tmp/")
        if cnstat_dir in file_list :
            if os.path.isfile("/tmp/{0}/{1}".format(cnstat_dir, uid)):
                with open( "/tmp/{0}/{1}".format(cnstat_dir, uid), 'rb') as readPtr:
                    count = 0
                    for line in readPtr:
                        EthernetObject = re.search(r"S\'(?P<ifname>\w+)\'",line.strip("\n "))
                        if EthernetObject is not None and EthernetObject.group("ifname") in EthernetList:
                            count = count +1
                            
                    if count == len(EthernetList):
                        notReady = False
                    else:
                        msg_to_syslog("info", "To clear and create the portstat file.")
                        os.system("portstat -c")
                        time.sleep(5)
            else:
                msg_to_syslog("info", "To clear and create the portstat file.")
                os.system("portstat -c")
                time.sleep(5)
        else:
            msg_to_syslog("info", "To clear and create the portstat file.")
            os.system("portstat -c")
            time.sleep(5)
        
    syslog.closelog()
if __name__ == "__main__":   
    main()
