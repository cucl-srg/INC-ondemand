#!/bin/bash
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



#########################################################
#   User Configuration                                  #
#########################################################
# Inc parameter
inc_thrld_lake_pps=10000000
inc_mode="--mode dynamic"
inc_upper_watt="40.0"
inc_upper_time_us="1000"
inc_lower_watt="30.0"
inc_lower_time_us="1000"

#########################################################
#  Network setting on Host A (Memcached with LaKe )     #
#########################################################
host_if="nf2"
host_ip="192.168.30.1"
host_mac="XX:XX:XX:XX:XX:XX" # Required to fill
lake_num_pe="5"

#########################################################
#  Network setting on Host B (Client)                   #
#########################################################
cli_host="nf-test205.nf.cl.cam.ac.uk"
cli_if="eth3"
cli_ip="192.168.30.2"
cli_mac="XX:XX:XX:XX:XX:XX" # Required to fill


#########################################################
#  Client software settings                             #
#########################################################
num_query=1000000
script_str0="./workload-gen -n ${num_query}  -s ${host_ip} -p 11211 -k uniform:4 -l uniform:8 -r 1 -u 0 -i fb_ia -f 4000 > latency.txt"

#########################################################
#   Constrain Configuration                             #
#########################################################

prog="/root/yuta/scripts/nf-memcached-test.sh"
rwaxi="inc-ondemand-controller/rwaxi"
inc_prog="inc-ondemand-controller/inc-ondemand.py"
inc_script="${inc_prog} ${inc_mode} --lake-threshold ${inc_thrld_lake_pps} \
             --lake-resol 1000000000 --network_th ${inc_upper_watt} \
             --time ${inc_upper_time_us} --host_th ${inc_lower_watt} \
              --verbose"

memcached="./memcached/memcached"

#########################################################
#   Functions                                           #
#########################################################
function cpu_stat {
	i=1
	time=$1
	file=$2
	if [ -z $file ]; then
		file="cpu.txt"
	fi
	while [ $i -le $time ]
	do 	
		date=`date +%s`
		usa=`ps aux | grep memcached | grep -v grep | awk '{print $3}'`
		echo "cpu ${data} ${usa}" > $file
		sleep 1 
		i=$(expr $i + 1)
	done
}

function tput_stat {
	i=1
	time=$1
	file=$2
	if [ -z $file ]; then
		file="tput.txt"
	fi
	while [ $i -le $time ]
	do
		data=`${rwaxi} -a 0x4406001C | xargs printf "%d\n"`
		date=`date +%s`
		echo "tput ${date} ${data}" > $file
		sleep 1
		i=$(expr $i + 1)
	done
}

#########################################################
#   Local machine                                       #
#########################################################
sudo killall memcached
# check the dependency
if [ ! -f $memcached ]; then
	echo "$memcached is not found"
	exit -1
fi
if [ ! -f $inc_prog ]; then
	echo "$inc_prog is not found"
	exit -1
fi

# server setting
echo -e "\e[31m[LaKe server] Loading sume_riffa...\e[m"
sudo modprobe sume_riffa
echo -e "\e[31m[LaKe server] Loading msr...\e[m"
sudo modprobe msr
echo -e "\e[31m[LaKe server] Disabling HyperThreading.\e[m"
echo 0 > /sys/devices/system/cpu/cpu4/online
echo 0 > /sys/devices/system/cpu/cpu5/online
echo 0 > /sys/devices/system/cpu/cpu6/online
echo 0 > /sys/devices/system/cpu/cpu7/online

echo -e "\e[31m[LaKe server] Disabling Turbo Boost.\e[m"
wrmsr -p0 0x1a0 0x4000850089
wrmsr -p1 0x1a0 0x4000850089
wrmsr -p2 0x1a0 0x4000850089
wrmsr -p3 0x1a0 0x4000850089
echo -e "\e[31m[Lake server] CPU Freq\e[m"
echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
echo performance > /sys/devices/system/cpu/cpu1/cpufreq/scaling_governor
echo performance > /sys/devices/system/cpu/cpu2/cpufreq/scaling_governor
echo performance > /sys/devices/system/cpu/cpu3/cpufreq/scaling_governor
echo -e "\e[31m[LaKe server] IP address setting ${host_ip} on ${host_if}\e[m"
sudo ifconfig ${host_if} up ${host_ip}
echo -e "\e[31m[LaKe server] ARP table registering ${cli_ip} ${cli_mac}\e[m"
sudo arp -s ${cli_ip} ${cli_mac}

# client setting
echo -e "\e[31m[KVS client] tuning...\e[m"
ssh root@${cli_host} "modprobe msr"
ssh root@${cli_host} "echo 0 > /sys/devices/system/cpu/cpu4/online"
ssh root@${cli_host} "echo 0 > /sys/devices/system/cpu/cpu5/online"
ssh root@${cli_host} "echo 0 > /sys/devices/system/cpu/cpu6/online"
ssh root@${cli_host} "echo 0 > /sys/devices/system/cpu/cpu7/online"
ssh root@${cli_host} "wrmsr -p0 0x1a0 0x4000850089"
ssh root@${cli_host} "wrmsr -p1 0x1a0 0x4000850089"
ssh root@${cli_host} "wrmsr -p2 0x1a0 0x4000850089"
ssh root@${cli_host} "wrmsr -p3 0x1a0 0x4000850089"
ssh root@${cli_host} "echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor"
ssh root@${cli_host} "echo performance > /sys/devices/system/cpu/cpu1/cpufreq/scaling_governor"
ssh root@${cli_host} "echo performance > /sys/devices/system/cpu/cpu2/cpufreq/scaling_governor"
ssh root@${cli_host} "echo performance > /sys/devices/system/cpu/cpu3/cpufreq/scaling_governor"

echo -e "\e[31m[KVS client] IP address setting ${cli_ip} on ${cli_if}\e[m"
ssh root@${cli_host} "ifconfig ${cli_if} ${cli_ip} up"
ssh root@${cli_host} "arp -s ${host_ip} ${host_mac}"

echo -e "\e[31m[LaKe server] ping to OSNT machine(${cli_ip}).\e[m"
ping -c 3 -i 0.2 ${cli_ip}

echo -e "\e[31m[LaKe server] PE setting (${lake_num_pe} PEs)\e[m"
${prog} pe-setting ${lake_num_pe}

echo -e "\e[31m[LaKE server] memcached starts\e[m"
${memcached} -U 11211 -B binary -t 4 -u root &

#########################################################
#   Ondemand Controller                                 #
#########################################################
echo -e "\e[31m[LaKe server] Starting on-demand controller setting.\e[m"
${inc_script} &
inc_pid=$!

echo -e "\e[31m[LaKe server] Done.\e[m"

#########################################################
#   OSNT machine                                        #
#########################################################
echo -e "\e[31m[KVS client] Starting packet injecting.\e[m"
{
	ssh root@${cli_host} "cd yuta/kvs-workload-gen/src; ${script_str0} "
} &

#########################################################
#   LaKe machine: Senario                               #
#########################################################
cpu_stat(60, "cpu.txt") &
tput_stat(50, "tput.txt") &

sleep 15
echo -e "\e[31m[LaKe server] Starting ChainerMN MPI script.\e[m"
cd /root/
mpiexec --allow-run-as-root -n 5 -hostfile host python chainermn/examples/mnist/train_mnist.py &
mpi_pid=$!

sleep 10

kill $mpi_pid

sleep 15

mpiexec --allow-run-as-root -n 5 -hostfile host python chainermn/examples/mnist/train_mnist.py &
mpi_pid=$!
sleep 15

kill $mpi_pid
sleep 5
#########################################################
#   LaKe machine                                        #
#########################################################
echo -e "\e[31m[LaKe server] killing INC ondemand controller.\e[m"
kill ${inc_pid}
echo -e "\e[31m[LaKe server] killing memcached.\e[m"
killall memcached
echo -e "\e[31m[LaKe server] Downloading pcap file.\e[m"
cd /root/yuta/inc-ondemand/test
scp root@$cli_host:~/path/to/kvs-workload-gen/src/latency.txt .

