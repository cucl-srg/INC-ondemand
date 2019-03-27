# Server Power Measurement

### Measurement
We used RAPL to measure power consumption.
We used power consumption monitor application.
```
https://github.com/deater/uarch-configure/tree/master/rapl-read
```

### Run measurement
In this script, we assumed that a machine has dual-socket Intel Xeon CPU (total 24 cores).

##### Download RAPL measurement monitor
```
# git clone https://github.com/deater/uarch-configure.git
```

### Load program
This load program is similar to 'yes' command.
You need to change macro variable NUM, 
depending on your machine with monitoring CPU usage.
```
# vim load.c
```
```
# vim Makefile
```

##### Run
```
# bash measure.sh
```

