import os
import syslog

DEBUG = False

def msg_to_syslog(level, msg):
    if level == 'info':
        syslog.syslog(syslog.LOG_INFO, msg)
    elif level == 'warning':
        syslog.syslog(syslog.LOG_WARNING, msg)
    elif level == 'debug' and DEBUG:
        syslog.syslog(syslog.LOG_DEBUG, msg)
