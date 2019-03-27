# INC on-demand Controller

### Requiremnet
 - msr (device driver)
 - axi codes NetFPGA-SUME (In this case)
 - memory map for each register on NIC (e.g., Packet Rate Threshold register)
 - RAPL(intel_rapl) driver support

If you encounter this message, you need to update the driver version.
```
intel_rapl: driver does not support CPU family 6 model 94
```
You can see the driver version (Linux kernel version) from this rapl_ids list.
For example, CPU model number 96(0x5E) is supported from Linux kernel 4.3.0.
https://elixir.bootlin.com/linux/v4.3/source/drivers/powercap/intel_rapl.c#L1089


### how to build and setup
To build,
```
$ make
```
To setup, 
```
$ sudo ./setup.sh
```

### Usage
```
$ ./inc-ondemand.py -h
usage: ./inc-ondemand.py <options>
    --show-settings, -s : Showing current settings.
    --verbose, -v       : verbose mode
    <NIC setting>
    --mode, -m          : 'on', 'off', 'dynamic'
    --lake-threshold, -l: LaKe Threshold (pps)
    --lake-resol, -e    : LaKe Resolution (ns)
    <Host setting>
    --network_th, -r    : Threshold shifting to NIC processing (Watt.)
    --network_time, -t  : The amount time shifting to NIC processing (usec)
    --host_th, -R       : Threshold shifting to host processing (Watt.)
    --host_time, -T     : The amount time shifting to host processing (usec)
```
