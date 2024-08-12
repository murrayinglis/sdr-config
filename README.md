# sdr-config

This project provides an interface to configure an Ettus USRP device from an xml file.\
Tests for SFCW, FMCW, Pulse, Compressed Pulse

## Command line interface usage:
### Flags:

* `--find` : finds and displays the address of all devices connected. 
* `--dump`: dump the config of a specified device to an xml file. 
Additionally, specify  the dump path with. 
* `--configure` : configure the device at a specific address based on a config xml file *(by default config.xml)*. 
Additionally, specify the config file path. 
* `--test` : run one of the tests.



## About
* The hardware RX_config.cpp and TX_config.cpp have methods for setting the:
    * Subdevice
    * Antenna
    * Sample rate
    * Bandwidth
    * Centre frequency - tune request for setting. NOTE: Set frequency and then check afterwards.
    * Gain
    * Analog frontend filter bandwidth
* The hardware RX_funcs.cpp and TX_funcs.cpp have methods for creating RX/TX streamers and transmitting their buffer/ receiver buffer and writing to a file. Buffer is in time domain. \
For now it has only been configured to use 1 channel? 1 streamer is configured per channel?
* Oscillator: 
    * Check for external or local (LO)
    * Check for oscillator locked (synchronised to precise frequency). This can take time.
    * TODO: mimo clock source?
    * Modulating/baseband signal mixed up to centre frequency by oscillator
* TODO mboard: find more info
    * Also have to synch oscillators
    * Sync times for RX USRP to mboard
* Wave considerations:
    * Check for out of Nyquist rate (wave frequency > TX rate/2)
    * Frequency too small for buffer
    * The waveform file should therefore have metadata containing freq, sample rate etc... to allow for these calculations
    * Uses an included waveform table class for generating specified waveforms AND should be able to read in arbitrary waveforms


## Tests
### Loopback
* Setup transmitter and receiver frequency and antenna parameters.
* Setup transmitter and receiver streamers
* Transmitter thread has a streamer which sends buffer until the receiver sets *stop signal* flag (static bool check). Receiver writes buffer to file.
* End of burst packet sent when stop signal received?
* Streamers shut down

### Proposed:
* Range resolution: SFCW v FMCW v ...


## DSP tuning - 2 stage tuning
Master CLK is multiplied to get to centre frequency. Since it is digital, it won't be exactly the centre freq probably. DSP tuning takes the TX signal and mixes it to get to desired centre freq. This introduces some phase distortion. Can be calculated for transmitting generic waveform - Dylan has functions for this. \
Additonally TX and RX use same MCLK. Phase distortion that cannot be removed.
