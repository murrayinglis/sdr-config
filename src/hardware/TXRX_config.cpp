#include "hardware.hpp"
#include <uhd/usrp/multi_usrp.hpp>
#include <chrono>

namespace hardware{

    void synhronousTXRXTune_basic(uhd::usrp::multi_usrp::sptr usrp, double txFreqHz, double rxFreqHz, double timeOffset){
        usrp->set_command_time(timeOffset);
        usrp->set_tx_freq(txFreqHz);
        uhd::tune_request_t rx_tune_req(rxFreqHz); 
        usrp->set_rx_freq(rx_tune_req);
        usrp->clear_command_time();

        //not doing any checking to see if oscillators locked
    }

    void synchronousTXRCTune_manual(uhd::usrp::multi_usrp::sptr usrp,double txFreqHz, double rxFreqHz, double timeOffset){
        uhd::tune_request_t txRequest(txFreqHz,1e6);
        uhd::tune_request_t rxRequest(rxFreqHz,1e6);

        uhd::tune_result_t txResult,rxResult;

        txRequest.dsp_freq_policy=uhd::tune_request_t::POLICY_MANUAL;
        txRequest.rf_freq_policy=uhd::tune_request_t::POLICY_AUTO;
        txRequest.dsp_freq=0;
        
        rxRequest.dsp_freq_policy=uhd::tune_request_t::POLICY_MANUAL;
        rxRequest.rf_freq_policy=uhd::tune_request_t::POLICY_AUTO;
        rxRequest.dsp_freq=0;


        usrp->set_command_time(timeOffset);
        txResult=usrp->set_tx_freq(txRequest);
        rxResult=usrp->set_rx_freq(rxRequest);
        usrp->clear_command_time();

        std::cout<<"TX Tune Result: "<<txResult.to_pp_string();
        std::cout<<"RX Tune Result: "<<rxResult.to_pp_string();

    }


    void synchronousTXRXIncrement(uhd::usrp::multi_usrp::sptr usrp, double incrementAmountHz, double timeOffset){
        double newTx, newRx;
        newTx=usrp->get_tx_freq()+incrementAmountHz;
        newRx=usrp->get_rx_freq()+incrementAmountHz;

        usrp->set_command_time(timeOffset);
        
        usrp->set_tx_freq(newTx);
        
        uhd::tune_request_t rx_tune_req(newRx); 
        usrp->set_rx_freq(rx_tune_req);
        
        usrp->clear_command_time();
    }


    bool checkTXRXLOsLocked(uhd::usrp::multi_usrp::sptr usrp, double timeout){
        auto startTime=std::chrono::high_resolution_clock::now();
        auto currentTIme=std::chrono::high_resolution_clock::now();

        std::chrono::high_resolution_clock::duration duration=currentTIme-startTime;

        while(double(duration.count()/1e9)<timeout){
            bool TXLocked, RXLocked=false;

            TXLocked=usrp->get_tx_sensor("lo_locked").to_bool();
            RXLocked=usrp->get_rx_sensor("lo_locked").to_bool();;

            if(TXLocked && RXLocked){
                return true;
            }
            
    
            currentTIme=std::chrono::high_resolution_clock::now();
            duration=currentTIme-startTime;
        }
        

        return false;

    }

    bool waitTilLocked(uhd::usrp::multi_usrp::sptr usrp, double timeout){
        auto startTime=std::chrono::high_resolution_clock::now();
        
        while(!usrp->get_tx_sensor("lo_locked").to_bool()||!usrp->get_rx_sensor("lo_locked").to_bool()){
            //do nothing
        }
        
        auto currentTime=std::chrono::high_resolution_clock::now();
        std::chrono::high_resolution_clock::duration duration = currentTime-startTime;
        return duration.count();

    }



} //namespace HARDWARE