#!/usr/bin/env python
# Copyright (c) 2019, Yuta Tokusashi
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
# * Neither the name of the project, the copyright holder nor the names of its
#  contributors may be used to endorse or promote products derived from
#   this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import sys, getopt
import os
import commands
import time
import pandas as pd

###########################################
#   Please modify in your environemnt     #
###########################################
Freq = 200.00 # MHz

###########################################
#   Constant variable                     #
###########################################

lake_reg_sw_addr = "0x440a002c"
lake_reg_host_sw_addr = "0x440a0030"
lake_reg_resol_addr = "0x440a0034"
lake_reg_thrld_addr = "0x440a0038"
path = ""
basic_cmd = "/rwaxi -a "
resol = 1000.00 / Freq # nano second
host_cmd_zero = ""
host_cmd_high = ""

df = pd.DataFrame(index=[], columns=['Power'])

def show_settings(mode=0, cresol=0, thrld=0, upper_bnd=0, lower_bnd=0):
    if mode == 0:
        print "    mode: Static off (Ignoring Network Rate)"
    elif mode == 1:
        print "    mode: Static on  (Ignoring Network Rate)"
    elif mode == 2:
        print "    mode: Dynamic"
    print "    LaKe Resolution: " + str(cresol) + " (ns)"
    print "    LaKe Threshold : " + str(thrld) + " (pps)"
    print "    Host Upper Threshold: " + str(upper_bnd) + " (Watt.)"
    print "    Host Lower Threshold: " + str(lower_bnd) + " (Watt.)"


def setting_lake(change=False, new_mode='dynamic', new_resol_ns=100000000, new_thrld_pps=1000000, upper_bnd=15.000, lower_bnd=20.000):
    cmd = path + basic_cmd + lake_reg_sw_addr + " | xargs printf \"%d\""
    current_mode_t = int(commands.getoutput(cmd))
    current_mode = int(commands.getoutput("echo $((" + str(current_mode_t) + " & 255))"))
    new_val_mode = current_mode
    time.sleep(0.5)
    cmd = path + basic_cmd + lake_reg_resol_addr
    current_resol = int(commands.getoutput(cmd), 16) * int(resol)
    new_val_resol = current_resol
    time.sleep(0.5)
    cmd = path + basic_cmd + lake_reg_thrld_addr
    current_thrld = int(commands.getoutput(cmd), 16) * int(resol)
    ew_val_thrld = current_thrld
    time.sleep(0.5)
    print "Current Settings:"
    show_settings(mode=current_mode, cresol=current_resol, thrld=current_thrld, upper_bnd=upper_bnd, lower_bnd=lower_bnd)

    if change:
        time.sleep(0.5)
        if new_mode == "off":
            new_val_mode = 0
        elif new_mode == "on":
            new_val_mode = 1
        elif new_mode == "dynamic":
            new_val_mode = 2
        cmd = path + basic_cmd + lake_reg_sw_addr + " -w " + str(new_val_mode)
        print cmd
        os.system(cmd)
        time.sleep(0.5)
        new_val_resol = new_resol_ns / resol
        cmd = path + basic_cmd + lake_reg_resol_addr + " -w " + str(new_val_resol)
        os.system(cmd)
        time.sleep(0.5)
        new_val_thrld = new_thrld_pps / resol
        cmd = path + basic_cmd + lake_reg_thrld_addr + " -w " + str(new_val_thrld)
        os.system(cmd)
        time.sleep(0.5)
        print "New Settings:"
        show_settings(new_val_mode, new_resol_ns, new_thrld_pps, upper_bnd, lower_bnd)
    
def power_driven_lake(onoff=False, verbose=False):
    if onoff:
        print host_cmd_high
        os.system(host_cmd_high)
        if verbose:
            print "Power-driven: on"
    else:
        print host_cmd_zero
        os.system(host_cmd_zero)
        if verbose:
            print "Power-driven: off"

def monitor_power(lower_bnd=15.000, upper_bnd=20.000, time=3, verbose=False):
    global df
    cmd = path + "/rapl-plot -t 1"
    res = commands.getoutput(cmd)
    power_str = res.split()
    time_p = power_str[0]
    print(power_str[1])
    power = float(power_str[1])
    series = pd.Series([power], index=df.columns)
    df = df.append(series, ignore_index = True)
    lll = df.rolling(window=3).mean()
    power_sma = lll.tail(1).iat[0,0]

    if power_sma < lower_bnd:
        #if host_to_time != 0.0:
        #    sleep_time = host_to_time / 1000.0
        #    time.sleep(sleep_time)
        power_driven_lake(onoff=False, verbose=verbose)
        if verbose:
            print "Lower " + time_p + " " + power_str[1] + " " + str(power_sma)
    elif upper_bnd < power_sma:
        #if ntwk_to_time != 0.0:
        #    sleep_time = ntwk_to_time / 1000.0
        #    time.sleep(sleep_time)
        #    #os.system("sleep " + str(sleep_time))
        power_driven_lake(onoff=True, verbose=verbose)
        if verbose:
            print "Upper " + time_p + " " +  power_str[1] + " " + str(power_sma)
    else:
        if verbose:
            print "Else " + time_p + " " + power_str[1] + " " + str(power_sma)

def usage():
    print 'usage: ./inc-ondemand.py <options>'
    print '    --show-settings, -s : Showing current settings.'
    print '    --verbose, -v       : verbose mode'
    print '    <NIC setting>'
    print '    --mode, -m          : \'on\', \'off\', \'dynamic\''
    print '    --lake-threshold, -l: LaKe Threshold (pps)'
    print '    --lake-resol, -e    : LaKe Resolution (ns)'
    print '    <Host setting>'
    print '    --network_th, -r    : Threshold shifting to NIC processing (Watt.)'
    print '    --network_time, -t  : The amount time shifting to NIC processing (usec)'
    print '    --host_th, -R       : Threshold shifting to host processing (Watt.)'
    print '    --host_time, -T     : The amount time shifting to host processing (usec)'
    print ''
    print ''

def main(argv):

    change = False
    verbose = False
    new_mode = 'dynamic'
    upper_bnd = 25.000 # Watt
    lower_bnd = 15.000 # Watt
    ntwk_to_time = 0.000 # micro second
    host_to_time = 0.000 # micro second
    lake_resol = 1000000000 # nano second
    lake_thrld = 1000000 # pps
    global path 
    global host_cmd_zero
    global host_cmd_high
    path = os.path.dirname(sys.argv[0])
    host_cmd_zero = path + basic_cmd + lake_reg_host_sw_addr + " -w 0"
    host_cmd_high = path + basic_cmd + lake_reg_host_sw_addr + " -w 1"
    #pathd = commands.getoutput("ls -d $PWD/" + sys.argv[0])
    #path = ${path%/*}

    try:
        opts, args = getopt.getopt(sys.argv[1:], "hr:t:R:sm:l:e:v", ["help", "mode=", "network_th=", "time=", "host_th=", "lake-threshold=", "lake-resol=", "show-settings", "verbose"])
    except getopt.GetoptError, err:
        print str(err)
        usage()
        sys.exit(2)

    for opt, arg in opts:
        if opt in ("-h", "--help"):
            usage()
            sys.exit()		
        elif opt in ("-r", "--network_th"):
            change = True
            upper_bnd = float(arg)
        elif opt in ("-t", "--time"):
            change = True
            ntwk_to_time = int(arg)
        elif opt in ("-R", "--host_th"):
            change = True
            lower_bnd = float(arg)
        elif opt in ("-l", "--lake-threshold"):
            change = True
            lake_thrld = int(arg)
        elif opt in ("-e", "--lake-resol"):
            change = True
            lake_resol = int(arg)
        elif opt in ("-s", "--show-settings"):
            setting_lake(change=False)
            sys.exit(0)
        elif opt in ("-m", "--mode"):
            change = True
            new_mode = arg
            if new_mode != "dynamic" and new_mode != "on" and new_mode != "off":
                sys.exit(1)
        elif opt in ("-v", "--verbose"):
            verbose=True

    setting_lake(change=change, new_mode=new_mode, new_resol_ns=lake_resol, new_thrld_pps=lake_thrld, upper_bnd=upper_bnd, lower_bnd=lower_bnd)

    try:
        while True:
            monitor_power(lower_bnd=lower_bnd, upper_bnd=upper_bnd, time=ntwk_to_time, verbose=verbose)
    except KeyboardInterrupt:
        sys.exit(1)


if __name__ == "__main__":
    main(sys.argv[1:])
