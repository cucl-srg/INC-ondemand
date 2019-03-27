# The Case For In-Network Computing On Demand: Reproduction Environment

This repository provides a reproduction environment for the paper "The Case For In-Network Computing On Demand" (EuroSys 2019).

When citing this work, please use the following citation information:
The Case For In-Network Computing On Demand, In ACM EuroSys 2019.
Authors: Yuta Tokusashi, Huynh Tu Dang, Fernando Pedone, Robert Soule and Noa Zilberman

# Website 
https://www.cl.cam.ac.uk/research/srg/netos/projects/inc-ondemand/

# Paper

[Official](https://dl.acm.org/citation.cfm?id=3303979)

[Open source version](https://www.repository.cam.ac.uk/handle/1810/289802)

# Paper Abstract
Programmable network hardware can run services traditionally deployed on servers, resulting in orders-of-magnitude improvements in performance. Yet, despite these performance improvements, network operators remain skeptical of in-network computing. The conventional wisdom is that the operational costs from increased power consumption outweigh any performance benefits. Unless in-network computing can justify its costs, it will be disregarded as yet another academic exercise. 

# Repository Structure
This repository allows to reproduce the figures presented in the paper, provides the source code used in the paper, and links to the source code of the projects referenced by this work (LaKe, P4xos and Emu).

* bitfiles - provides links to the bitfiles used in this work. The bitfile can not reside within this repo for licensing reasons.

* data - the data used to generate figures 3 to 7 in the paper.

* DNS - includes the pcap trace file and the commands used to drive the Emu DNS design. See README.md within the folder.

* kvs - includes all the information required for KVS and LaKe-based experiments.  See README.md within the folder. The folder includes the following subfolders:
  * inc-ondemand - the in-network computing on-demand controller (host-based)
  * kvs-workload-gen - Key-Value store workload generator
  * pcap - pcap trace files used in some of the experiments

* paxos - includes all the information required for paxos related experiments.  See README.md within the folder. The folder includes the following subfolders:  
  * pcap -  pcap trace files used in some of the experiments
  * scripts  - scripts for running different paxos roles 

# Projects source code

The source code for LaKe and Emu DNS is part of the upcoming [NetFPGA SUME](https://github.com/NetFPGA/NetFPGA-SUME-live) release (April / 2019).

The source code for P4xos will be made available under the [P4-NetFPGA](https://github.com/NetFPGA/P4-NetFPGA-live) project. 
