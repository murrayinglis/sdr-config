# sdr-config

This project provides an interface to configure an Ettus USRP device from an xml file.\
Tests for SFCW, FMCW, Pulse, Compressed Pulse.\
Submodules not setup. CMakeLists.txt has lines which are system specific.

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
    * more ...
* The hardware RX_funcs.cpp and TX_funcs.cpp have methods for creating RX/TX streamers and transmitting their buffer/ receiver buffer and writing to a file. Buffer is in time domain. \
For now it has only been configured to use 1 channel? 1 streamer is configured per channel for per RX/TX.
* Oscillator: 
    * Check for external / local (LO) / PPS
    * __Known issue for N2xx series__: When setting clock_source to external, but not providing an external reference, there are cases when a successful lock is reported even though no clock signal is present, which can cause false positives. Terminating the clock input may help in some cases. 
    * Check for oscillator locked (synchronised to precise frequency). This can take time.
    * TODO: mimo clock source?
    * Modulating/baseband signal mixed up to centre frequency by oscillator
    * Phase alignment
* TODO mboard: find more info
    * Also have to synch oscillators
    * Sync times for RX USRP to mboard
* Wave considerations:
    * Check for out of Nyquist rate (wave frequency > TX rate/2)
    * Frequency too small for buffer
    * The waveform file should therefore have metadata containing freq, sample rate etc... to allow for these calculations
    * Uses an included waveform table class for generating specified waveforms AND should be able to read in arbitrary waveforms
* Recommended testing procedures: https://files.ettus.com/manual/page_rdtesting.html


## Tests
### RX with Sig Gen
[x] eg centre freq at 900MHz and transmit 900.1MHz, expect 100kHz beat in fft \
[] get rx test to perform fft and find beat freq \
[x] loopback test \
[x] arbitrary waveform read in and tx \
[x] arbitrary waveform loopback \
[] latency test (working to radar) \


### Loopback
* Setup transmitter and receiver frequency and antenna parameters.
* Setup transmitter and receiver streamers
* Transmitter thread has a streamer which sends buffer until the receiver sets *stop signal* flag (static bool check). Receiver writes buffer to file.
* End of burst packet sent when stop signal received?
* Streamers shut down

### Proposed:
* Range resolution: SFCW v FMCW v ...


## DSP tuning - 2 stage tuning
Master CLK is fixed for usrp2. Local oscillator (LO) is tuned by daughter board. Master CLK is multiplied to get to centre frequency. Since it is digital, it won't be exactly the centre freq probably. DSP tuning takes the TX signal and mixes it to get to desired centre freq. This introduces some phase distortion. Can be calculated for transmitting generic waveform - Dylan has functions for this. \
Additonally TX and RX use same MCLK. Phase distortion that cannot be removed.\
tune_result_t and tune_request_t policy for RF and DSP frequency \
https://files.ettus.com/manual/page_sync.html \
Additionally, the signal chain can be customised within the FPGA by inserting custom DSP modules.


## Streaming
For N200 series, only the addr and port arguments are supported.

* L - Late. Sent samples to USRP with timespec but time has already passed 
* U - Underrun (send not called often enough for TX)
* O - Overrun (recv not called often enough for RX)

Converters: Most FPGAs use integer data types, the most common being complex 16-bit integers. This is converted casts, scales and handles endianness. The format used by the user application is coined the 'CPU Format', whereas the format used between the device and UHD is called the 'OTW Format' (Over-the-wire format). A custom converter can be defined. - This is not explored, but good to mention in the report. For the purposes of this project, only `double` type is used.

__Note:__ Changing the OTW format can have side effects. For example, using an OTW format with fewer bits (sc8 vs. sc16, for instance) reduces the load on the data link and allows more bandwidth, but also reduces the dynamic range of the data and increases quantization noise. 


__Queuing commands:__ I want to try queueing commands in the future to set up an actual pulsed radar, instead of the pulsed CW radar I am using. I can then packetize every received frame instead of appending it to one large file.


## Note on RFNoc blocks
This is a more granular API than the Multi-USRP API, and can directly interact with RFNoC blocks in a device, and the RFNoC graph. This is available for all Generation-3 USRPs and above. https://files.ettus.com/manual/page_coding.html \
This is therefore outside the scope here for now (using gen 2).




## After recording processing:
Matched filter to get time delay. For these parameters I am getting a delay between tx and rx threads of about 500us. By using the transmitted signal that is present in the receive channel due to the limited isolation, a timestamp of t=0 is obtained, automatically accounting for the timing offset between the Tx and Rx channels.