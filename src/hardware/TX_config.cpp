#include "hardware.hpp"
#include <uhd/usrp/multi_usrp.hpp>
#include "config.hpp"


namespace hardware{
    bool confirmTxOscillatorsLocked(uhd::usrp::multi_usrp::sptr usrp_object, std::string ref_source, bool printing){
        std::string clock_source = ref_source;
        std::vector<std::string> tx_sensor_names;
        tx_sensor_names = usrp_object->get_tx_sensor_names(0);

        // checking LO
        if (std::find(tx_sensor_names.begin(), tx_sensor_names.end(), "lo_locked")!= tx_sensor_names.end()){
            uhd::sensor_value_t lo_locked = usrp_object->get_tx_sensor("lo_locked", 0);
            if(printing){std::cout << boost::format("Checking TX.... %s ...") % lo_locked.to_pp_string()  << std::endl;}
            if(!lo_locked.to_bool()){
                return false;
            }
        }
        
        tx_sensor_names = usrp_object->get_mboard_sensor_names(0);
        if ((clock_source == "mimo") and (std::find(tx_sensor_names.begin(), tx_sensor_names.end(), "mimo_locked")!= tx_sensor_names.end())) {
            uhd::sensor_value_t mimo_locked = usrp_object->get_mboard_sensor("mimo_locked", 0);
            if(printing){std::cout << boost::format("Checking TX .... %s ...") % mimo_locked.to_pp_string()<< std::endl;}
            if(!mimo_locked.to_bool()){
                return false;
            }
        }
        if ((clock_source == "external") and (std::find(tx_sensor_names.begin(), tx_sensor_names.end(), "ref_locked")!= tx_sensor_names.end())) {
            uhd::sensor_value_t ref_locked = usrp_object->get_mboard_sensor("ref_locked", 0);
            if(printing){std::cout << boost::format("Checking TX .... : %s ...") % ref_locked.to_pp_string() << std::endl;}
            if(!ref_locked.to_bool()){
                return false;
            }
        }
        return true;
    }

    int setupTransmitter(uhd::usrp::multi_usrp::sptr tx_usrp){
        //ref clock already set up
        tx_usrp->set_tx_subdev_spec(config::TX_SUBDEV);
        tx_usrp->set_tx_antenna(config::TX_ANTENNA);


        // sample rate
        double tx_rate=config::TX_RATE;
        std::cout << "Setting TX Rate (MHz):  "<< (tx_rate / 1e6)<< std::endl;
        tx_usrp->set_tx_rate(tx_rate);
        double actualRate=tx_usrp->get_tx_rate();
        if (tx_rate=!actualRate){
            std::cout << "Actual TX Rate (MHz) : "<< (actualRate / 1e6)<<". (Overwritten config) n";
            config::TX_RATE=actualRate;
        }
        
        
        // bandwidth
        tx_usrp->set_tx_bandwidth(config::TX_BW);
        // center freq
        double tx_center_freq= config::TX_FREQ;
        hardware::setTxFreqHz(tx_usrp,tx_center_freq);
        // gain
        double tx_gain = config::TX_GAIN;
        std::cout << "Setting TX Gain (dB) : " << tx_gain<< std::endl;
        tx_usrp->set_tx_gain(tx_gain);
        std::cout << "Actual TX Gain (dB) : " << tx_usrp->get_tx_gain() <<". Out of a possible range:"<< tx_usrp->get_tx_gain_range().to_pp_string()<< std::endl;
        
        //tx_usrp->set_rx_dc_offset(false);
        
        // make sure LO locked (give it a few attempts)
        size_t numlockAttempts=0;
        while( numlockAttempts<5&&!confirmTxOscillatorsLocked(tx_usrp,config::REF_CLOCK,true)){
            numlockAttempts++;
        }
        // 
        return 0;


    }

    bool setTxFreqHz(uhd::usrp::multi_usrp::sptr tx_usrp, double newTxFreqHz){

        tx_usrp->set_tx_freq(newTxFreqHz);

        if((std::abs(tx_usrp->get_tx_freq()-newTxFreqHz)>100)){ // if more than 100Hz off 
            std::cerr<<"setting of center freq unsuccessful. Requested: "<< (double)newTxFreqHz/1e6<<" Error: "<<tx_usrp->get_tx_freq()-newTxFreqHz<<"\n";    
        }

        size_t numlockAttempts=0;
        while( numlockAttempts<5&&!confirmTxOscillatorsLocked(tx_usrp,config::REF_CLOCK,false)){
            numlockAttempts++;
        }

        if(numlockAttempts>4){
            std::cerr<<"TX Did not lock in time\n";
            return false;
        }else{
            return true;
        }
    }



    bool incrementTxFreqHz(uhd::usrp::multi_usrp::sptr tx_usrp, double freqIncHz){
        double newFreq = tx_usrp->get_tx_freq()+freqIncHz;
        if(newFreq<config::MIN_FREQ){
            std::cerr<<"Requested Frequency Lower than SBX board capable of. If not using SBX, edit config_constants.hpp\n";
            return false;
        }
        if(newFreq>config::MAX_FREQ){
            std::cerr<<"Requested Frequency higher than SBX board capable of. If not using SBX, edit config_constants.hpp\n";
            return false;
        }

        return setTxFreqHz(tx_usrp, newFreq);

    }

}//namespace HARDWARE