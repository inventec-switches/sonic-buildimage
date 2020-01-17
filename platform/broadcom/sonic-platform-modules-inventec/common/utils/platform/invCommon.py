#!/usr/bin/python
import sys
import os
import re


class InvPlatform():
    # define board type
    inv_magnolia        = "x86_64-inventec_d6254qs-r0"
    inv_redwood         = "x86_64-inventec_d7032q28b-r0"
    inv_cypress         = "x86_64-inventec_d7054q28b-r0"
    inv_maple           = "x86_64-inventec_d6356-r0"
    inv_maple_j         = "x86_64-inventec_d6356j-r0"
    inv_maple_evt1      = "x86_64-inventec_d6556-r0"
    inv_sequoia         = "x86_64-inventec_d7264q28b-r0"


    def get_platform(self):
        platform = None

        fileP = "/host/machine.conf"
        with open("{0}".format(fileP), 'rb') as content:
            for line in content:
                reObj = re.search(r"onie_platform=(?P<pName>.+)", line)
                if reObj is not None:
                    platform = reObj.group("pName")

        return platform
