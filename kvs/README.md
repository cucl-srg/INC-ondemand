# Memcached (LaKe) experiment

## Install dependencies 
```
$ ./install.sh
```

## Figure.3 in EuroSys paper

### Server Side
###### Download bitfile to FPGA
```
$ xmd
XMD% fpga -f LaKe-5pe.bit
```
##### Network setting
Please connect cable with port 0 on NetFPGA-SUME.
```
$ sudo ifconfig nf0 192.168.70.1 up
$ sudo arp -s 192.168.70.2 e4:1d:2d:00:c4:30
```
##### Run memcached
```
$ ./memcached/memcached -U 11211 -B binary -t 4 
```

### Client setup
##### Download bitfile to FPGA
```
$ xmd
XMD% fpga -f osnt.bit
$ sudo reboot
```
##### Run OSNT gui
```
$ sudo modprobe sume_riffa
$ cd /path/to/OSNT-SUME-live/projects/osnt/sw/host/app/gui/
$ sudo python generator_gui.py
```

##### Procedure
```
1. Please set set.pcap to write a data to cache and inject it with just once repeating.
2. Please set IPG to vary throughput and measure power consumption by using a power meter.
```


## Figure.6 in EuroSys paper

##### Place kvs-workload-gen on client machine.
```
$ scp -r kvs-workload-gen <client machine>
```

##### Edit the script
Please edit this script for server/client configuration.
Especially, host names of server/client and MAC address 
on each NIC.
```
$ vim test-transition.sh
```

##### Download bitfile to FPGA
```
$ xmd
XMD% fpga -f LaKe-ondemand.bit
```

Please execute this script in the server side.
```
$ sudo ./test-transition.sh
```

