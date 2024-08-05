#include "hardware.hpp"
#include "config.hpp"
#include <uhd/usrp/multi_usrp.hpp>

namespace hardware{

    bool setRXFreqHz(uhd::usrp::multi_usrp::sptr rx_usrp, double newRXFreqHz){
        uhd::tune_request_t rx_tune_req(newRXFreqHz); 
        rx_usrp->set_rx_freq(rx_tune_req);

        if(std::abs(rx_usrp->get_rx_freq()-newRXFreqHz)>100){
            std::cerr<<"setting of center freq unsuccessful. Requested: "<< (double)newRXFreqHz/1e6<<" Error: "<<rx_usrp->get_rx_freq()-newRXFreqHz<<"\n";    
        }
        
        return true;
    }

    /**
     * Confirm LO, MIMO and REF clock sources locked
     * 
     */
    bool confirmRxOscillatorsLocked(uhd::usrp::multi_usrp::sptr usrp_object, std::string ref_source,bool printing){
        std::string clock_source = ref_source;
        std::vector<std::string> rx_sensor_names;
        //checking LO
        rx_sensor_names = usrp_object->get_rx_sensor_names();
        if (std::find(rx_sensor_names.begin(), rx_sensor_names.end(), "lo_locked")!= rx_sensor_names.end()){
            uhd::sensor_value_t lo_locked = usrp_object->get_rx_sensor("lo_locked");
            if(printing){std::cout << boost::format("Checking RX.... %s ...") % lo_locked.to_pp_string()  << std::endl;}
            if(!lo_locked.to_bool()){
                return false;
            }
        }
        //checking for mimo and external
        rx_sensor_names = usrp_object->get_mboard_sensor_names();
        if ((clock_source == "mimo") and (std::find(rx_sensor_names.begin(), rx_sensor_names.end(), "mimo_locked")!= rx_sensor_names.end())) {
            uhd::sensor_value_t mimo_locked = usrp_object->get_mboard_sensor("mimo_locked", 0);
            if(printing){std::cout << boost::format("Checking RX .... %s ...") % mimo_locked.to_pp_string()<< std::endl;}
            if(!mimo_locked.to_bool()){
                return false;
            }
        }
        if ((clock_source == "external") and (std::find(rx_sensor_names.begin(), rx_sensor_names.end(), "ref_locked")!= rx_sensor_names.end())) {
            uhd::sensor_value_t ref_locked = usrp_object->get_mboard_sensor("ref_locked", 0);
            if(printing){std::cout << boost::format("Checking RX .... : %s ...") % ref_locked.to_pp_string() << std::endl;}
            if(!ref_locked.to_bool()){
                return false;
            }
        }
        return true;
    }

    
    int setupReceiever(uhd::usrp::multi_usrp::sptr rx_usrp){
        // clock should already be set up for whole device
        rx_usrp->set_rx_subdev_spec(config::RX_SUBDEV);
        rx_usrp->set_rx_antenna(config::RX_ANTENNA);
        
        // sample rate
        double rx_rate=config::RX_RATE;
        std::cout << "Setting RX Rate (MHz):  "<< (rx_rate / 1e6)<< std::endl;
        rx_usrp->set_rx_rate(rx_rate);
        double actualRate=rx_usrp->get_rx_rate();
        if (rx_rate=!actualRate){
            std::cout << "Actual RX Rate (MHz) : "<< (actualRate / 1e6)<<". (Overwritten config) n";
            config::RX_RATE=actualRate;
        }



        // bandwidth
        std::cout << "Setting RX bandwidth (MHz):   " << (config::RX_BW / 1e6)  << std::endl;
        rx_usrp->set_rx_bandwidth(config::RX_BW);
        double actualBW = rx_usrp->get_rx_bandwidth();
        if (config::RX_BW != actualBW){
            std::cout << "Actual RX bandwidth (MHz) : "<< (actualBW / 1e6)<<". (Overwritten config) n";
            config::RX_RATE=actualRate;
        }    


        // center freq
        double rx_center_freq= config::RX_FREQ;
        std::cout << "Setting RX center freq (MHz): " << rx_center_freq / 1e6 << std::endl;
        setRXFreqHz(rx_usrp,rx_center_freq);



        // gain
        double rx_gain = config::RX_GAIN;
        std::cout << "Setting RX Gain (dB) : " << rx_gain<< std::endl;
        rx_usrp->set_rx_gain(rx_gain);
        std::cout << "Actual RX Gain (dB) : " << rx_usrp->get_rx_gain() <<". Out of a possible range:"<< rx_usrp->get_rx_gain_range().to_pp_string() << std::endl;


        // make sure LO locked (give it a few attempts)
        size_t numlockAttempts=0;
        while( numlockAttempts<5&&!confirmRxOscillatorsLocked(rx_usrp,config::REF_CLOCK,true)){
            numlockAttempts++;
        }
        std::vector<std::string> rx_sensor_names = rx_usrp->get_tx_sensor_names(0);
        if (std::find(rx_sensor_names.begin(), rx_sensor_names.end(), "lo_locked")!= rx_sensor_names.end()){
            uhd::sensor_value_t lo_locked = rx_usrp->get_tx_sensor("lo_locked", 0);
            if(!lo_locked.to_bool()){
                std::cout << "LO failed to lock in time." << std::endl;
            }
            else
            {
                std::cout << "LO locked in time." << std::endl;
            }
        }
        // 
        rx_sensor_names = rx_usrp->get_mboard_sensor_names(0);
        if ((config::REF_CLOCK == "mimo") and (std::find(rx_sensor_names.begin(), rx_sensor_names.end(), "mimo_locked")!= rx_sensor_names.end())) {
            uhd::sensor_value_t lo_locked = rx_usrp->get_tx_sensor("mimo_locked", 0);
            if(!lo_locked.to_bool()){
                std::cout << "MIMO failed to lock in time." << std::endl;
            }
            else
            {
                std::cout << "MIMO locked in time." << std::endl;
            }
        }
        //
        rx_sensor_names = rx_usrp->get_mboard_sensor_names(0);
        if ((config::REF_CLOCK == "external") and (std::find(rx_sensor_names.begin(), rx_sensor_names.end(), "ref_locked")!= rx_sensor_names.end())) {
            uhd::sensor_value_t lo_locked = rx_usrp->get_tx_sensor("lo_locked", 0);
            if(!lo_locked.to_bool()){
                std::cout << "External clock failed to lock in time." << std::endl;
            }
            else
            {
                std::cout << "External clock locked in time." << std::endl;
            }
        }
        std::cout << std::endl;



        //required for non-modulated waveforms. (Rory, you should probably remove this)
        rx_usrp->set_rx_dc_offset(false);

        
        if (rx_usrp->get_num_mboards() > 1) {
            rx_usrp->set_time_unknown_pps(uhd::time_spec_t(0.0));
        }
        
        // 
        return 0;
        
    } //setupReceiver

    bool incrementRxFreq(uhd::usrp::multi_usrp::sptr rx_usrp, double incrementFreqHz){
        double newFreq = rx_usrp->get_rx_freq()+incrementFreqHz;
        if(newFreq<config::MIN_FREQ){
            std::cerr<<"Requested Frequency Lower than SBX board capable of. If not using SBX, edit config_constants.hpp\n";
            return false;
        }
        if(newFreq>config::MAX_FREQ){
            std::cerr<<"Requested Frequency higher than SBX board capable of. If not using SBX, edit config_constants.hpp\n";
            return false;
        }

        return setRXFreqHz(rx_usrp,newFreq);
    }// incrementRxFreq()
    

}//namespace HARDWARE