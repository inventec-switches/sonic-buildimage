# snmputil.py
#
# Platform-specific SNMP interface for SNMP SetRequest
#

try:
    import os
    import re
    import syslog
    import subprocess
except ImportError as e:
    raise ImportError("%s - required module not found" % str(e))

class SnmpUtil(object):

    setreq_queue = []

    supported_oids = {} 
    portname_map = {}

    def load_port_config(self):
        filepath = '/usr/share/sonic/hwsku/port_config.ini'
        with open(filepath) as fp:
            for line in fp:
                if re.match(r'^#', line) is None:
                    f = line.split()
                    self.portname_map[str(int(f[3])+1)] = f[0]

    # execute a command in shell and return the output.
    def execute(self, asCommand, bSplitLines=False, bIgnoreErrors=False):
        p = subprocess.Popen(asCommand,
                             stdin=subprocess.PIPE,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.STDOUT,
                             shell=True,
                             close_fds=True,
                             universal_newlines=True,
                             env=None)
        if bSplitLines:
            printout = p.stdout.readlines()
        else:
            printout = p.stdout.read()
        '''
        iRetCode = p.wait()
        if iRetCode and not bIgnoreErrors:
            print("[INFO]Failed to execute git command")
        '''
        return printout

    # ifAdminStatus (.1.3.6.1.2.1.2.2.1.7)
    def config_admin_status(self, oid, val):
        updown = [ '', 'up', 'down' ]
        portname = self.portname_map[oid[-1]]
        command = "/sbin/ip link set dev {} {}".format(portname, updown[int(val)])
        self.execute(command)

    def construct_supported_oids(self):
        # ifAdminStatus
        self.supported_oids['.1.3.6.1.2.1.2.2.1.7'] = self.config_admin_status

    def __init__(self):
        print("[SnmpUtil.__init__]")
        self.load_port_config()
        self.construct_supported_oids()

    # handle Agent-X TestSet request
    def testset(self, oid, val):
        is_supported = False
        for _oid in self.supported_oids.keys():
            if _oid in oid:
                is_supported = True
                self.setreq_queue.append((self.supported_oids[_oid], oid, val))
        
        return is_supported       

    # handle Agent-X CommitSet request
    def commitset(self):
        for (func, oid, val) in self.setreq_queue:
            func(oid, val)

    # handle Agent-X CleanUpSet request
    def cleanupset(self):
        del self.setreq_queue[:]

