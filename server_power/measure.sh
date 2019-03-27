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


prog="./uarch-configure/rapl-read/rapl-plot"

echo "power consumption of IDLE (30sec)"
${prog} | awk '{print $2 " " $7 " " $2 + $7}'  &
pid_num=$!
sleep 30

kill $pid_num

sleep 3

echo "power consumption of All packages (30sec)"
for i in `seq 0 27`
do
	taskset -c $i yes > /dev/null &
done
${prog} | awk '{print $2 " " $7 " " $2 + $7}'  &
pid_num=$!
sleep 30

kill $pid_num
killall yes

sleep 3

echo "power consumption of Single package (30sec)"
for i in `seq 0 13`
do
	taskset -c $i yes > /dev/null &
done
${prog} | awk '{print $2 " " $7 " " $2 + $7}'  &
pid_num=$!
sleep 30

kill $pid_num
killall yes

sleep 3

echo "power consumption of Single core (30sec)"
taskset -c $i yes > /dev/null &
${prog} | awk '{print $2 " " $7 " " $2 + $7}'  &
pid_num=$!
sleep 30

kill $pid_num
killall yes

sleep 3


echo "power consumption of 2 cores (30sec)"
for i in `seq 0 1`
do
	taskset -c $i yes > /dev/null &
done
${prog} | awk '{print $2 " " $7 " " $2 + $7}'  &
pid_num=$!
sleep 30

kill $pid_num
killall yes

sleep 3

echo "power consumption of 3 cores (30sec)"
for i in `seq 0 2`
do
	taskset -c $i yes > /dev/null &
done
${prog} | awk '{print $2 " " $7 " " $2 + $7}'  &
pid_num=$!
sleep 30

kill $pid_num
killall yes

sleep 3

echo "power consumption of 4 cores (30sec)"
for i in `seq 0 3`
do
	taskset -c $i yes > /dev/null &
done
${prog} | awk '{print $2 " " $7 " " $2 + $7}'  &
pid_num=$!
sleep 30

kill $pid_num
killall yes

sleep 3

echo "power consumption of 2 cores (Different socket) (30sec)"
taskset -c 0 yes > /dev/null &
taskset -c 14 yes > /dev/null &
${prog} | awk '{print $2 " " $7 " " $2 + $7}'  &
pid_num=$!
sleep 30

kill $pid_num
killall yes

sleep 3



echo "power consumption of 2 cores (Different socket) 10 percentage (30sec)"
taskset -c 0 ./load-10p > /dev/null &
taskset -c 14 ./load-10p > /dev/null &
${prog} | awk '{print $2 " " $7 " " $2 + $7}'  &
pid_num=$!
sleep 30

kill $pid_num
killall ./load-10p

sleep 3



echo "power consumption of 2 cores 10 percentage (30sec)"
for i in `seq 0 1`
do
	taskset -c $i ./load-10p > /dev/null &
done
${prog} | awk '{print $2 " " $7 " " $2 + $7}'  &
pid_num=$!
sleep 30

kill $pid_num
killall ./load-10p

sleep 3


echo "power consumption of a core 10 percentage (30sec)"
taskset -c 0 ./load-10p > /dev/null &
${prog} | awk '{print $2 " " $7 " " $2 + $7}'  &
pid_num=$!
sleep 30

kill $pid_num
killall ./load-10p

sleep 3



echo "power consumption of a core 20 percentage (30sec)"
for i in `seq 0 0`
do
	taskset -c $i ./load-20p > /dev/null &
done
${prog} | awk '{print $2 " " $7 " " $2 + $7}'  &
pid_num=$!
sleep 30

kill $pid_num
killall ./load-20p

sleep 3


echo "power consumption of a core 50 percentage (30sec)"
for i in `seq 0 0`
do
	taskset -c $i ./load-50p > /dev/null &
done
${prog} | awk '{print $2 " " $7 " " $2 + $7}'  &
pid_num=$!
sleep 30

kill $pid_num
killall ./load-50p

sleep 3

echo "power consumption of Single package (30sec) at 10%"
for i in `seq 0 13`
do
	taskset -c $i ./load-10p > /dev/null &
done
${prog} | awk '{print $2 " " $7 " " $2 + $7}'  &
pid_num=$!
sleep 30

kill $pid_num
killall load-10p

sleep 3


echo "power consumption of Single package (30sec) at 20%"
for i in `seq 0 13`
do
	taskset -c $i ./load-20p > /dev/null &
done
${prog} | awk '{print $2 " " $7 " " $2 + $7}'  &
pid_num=$!
sleep 30

kill $pid_num
killall load-20p

sleep 3


echo "power consumption of Single package (30sec) at 50%"
for i in `seq 0 13`
do
	taskset -c $i ./load-50p > /dev/null &
done
${prog} | awk '{print $2 " " $7 " " $2 + $7}'  &
pid_num=$!
sleep 30

kill $pid_num
killall load-50p

sleep 3


