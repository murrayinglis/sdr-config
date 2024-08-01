# sdr-config

This project provides an interface to configure an Ettus USRP device from an xml file.\
Tests for SFCW, FMCW, Pulse, Compressed Pulse.\
Submodules not setup. CMakeLists.txt has lines which are system specific.

## Command line interface usage:
### Flags:

* `-find` : finds and displays the address of all devices connected. 
* `-dump`: dump the config of a specified device to an xml file. 
Additionally, specify  the dump path with. 
* `-configure` : configure the device at a specific address based on a config xml file *(by default config.xml)*. 
Additionally, specify the config file path. 

