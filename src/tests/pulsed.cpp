
// This software is modified by Murray Inglis for his final year thesis
//
// This software is based on the Ettus Research Benchmark rate example provided with the UHDdriver.
// it has been heavily modified by Marko Slijepcevic to facilitate the Radar application.
// Copyright 2011-2013 Ettus Research LLC
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include "tests.hpp"
#include "utils.hpp"
#include "hardware.hpp"
#include "wfm.h"
#include "MTimer.h"
#include <easyBMP/EasyBMP.h>
#include "easyBMP/EasyBMP_BMP.h"

#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/utils/thread.hpp>

#include <iostream>
#include <fstream>
#include <filesystem>
#include <sys/time.h>
#include <sys/resource.h>

#include <boost/thread/thread.hpp>
#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

namespace tests
{
    namespace pulsed
    {
        int counter = 0;
        unsigned long long num_overflows = 0;
        unsigned long long num_underflows = 0;
        unsigned long long num_rx_samps = 0;
        unsigned long long num_tx_samps = 0;
        unsigned long long num_dropped_samps = 0;
        unsigned long long num_seq_errors = 0;
        unsigned int generateHeatMapValue(double value)
        {
            if(value > 1)value = 1;
            if(value < 0)value = 0;
            //black, blue, green, yellow,orange,red,white
            const float
            colorMap[7][3]={{0,0,0},{0,0,255},{0,255,0},{255,255,0},{255,165,0},{255,0,0},{255,255,255}};
            value = value*6;// scale to the size of the color map
            int colderColor = floor(value);
            int hotterColor = colderColor +1;
            if(hotterColor > 6)hotterColor = 6;// only happens if value is exactly 1
            unsigned int R = colorMap[colderColor][0]+(value-
            colderColor)*(colorMap[hotterColor][0]-colorMap[colderColor][0]);// interpolation, red
            unsigned int G = colorMap[colderColor][1]+(value-
            colderColor)*(colorMap[hotterColor][1]-colorMap[colderColor][1]);// interpolation, green
            unsigned int B = colorMap[colderColor][2]+(value-
            colderColor)*(colorMap[hotterColor][2]-colorMap[colderColor][2]);// interpolation, blue
            unsigned int res = (R << 16) + (G<<8) + B;
            return res;
        }

        RGBApixel convertToRGBApixel(unsigned int color) 
        {
            RGBApixel pixel;

            // Extract Red, Green, Blue, and Alpha values from the color
            pixel.Red = (color >> 16) & 0xFF;   // Extract Red component (bits 16-23)
            pixel.Green = (color >> 8) & 0xFF;  // Extract Green component (bits 8-15)
            pixel.Blue = color & 0xFF;          // Extract Blue component (bits 0-7)

            // If the color includes an alpha channel (0xAARRGGBB)
            // Extract Alpha component (bits 24-31)
            // If not, you can set Alpha to 255 (fully opaque)
            pixel.Alpha = (color >> 24) & 0xFF; // Extract Alpha component (bits 24-31)

            return pixel;
        }

        void txFunc(
            uhd::usrp::multi_usrp::sptr usrp,
            const std::string &tx_cpu,
            uhd::tx_streamer::sptr tx_stream,
            double freq,
            double time,
            int numSamps,
            wfm tx1,
            wfm tx2,
            int reps
        ) 
        {
            uhd::set_thread_priority_safe();
            std::vector< complex<float> > buff(numSamps);
            std::vector< complex<float> > buff2(numSamps);
            // going to start with 10mS repetition rate, may make it variable in future
            double interval = 10e-3;

            for(int i = 0; i < numSamps; i++)
            {
                buff[i] = complex<float> (tx1[i].real(),tx1[i].imag());
                buff2 [i]= complex<float> (tx2[i].real(),tx2[i].imag());
            }
            std::vector<const void *> Txbuffs;
            Txbuffs.push_back(&buff.front());
            Txbuffs.push_back(&buff2.front());

            uhd::tx_metadata_t md;
            md.time_spec = time;
            md.has_time_spec = true;
            uhd::time_spec_t theTime;
            theTime = usrp->get_time_now();
            uhd::async_metadata_t asmd;

            int num_tx_samps = 0;
            for( int i = 0; i < reps; i++)
            {
                md.start_of_burst = true;
                md.end_of_burst = true;
                // TODO: implement max num samps
                num_tx_samps = tx_stream->send(Txbuffs, numSamps, md, 0.5)*tx_stream->get_num_channels();
                while(usrp->get_time_now().get_real_secs() < (md.time_spec.get_real_secs()+(tx1.size()/tx1.Fs)))
                {
                    boost::this_thread::sleep(boost::posix_time::microseconds(500));
                }
                md.time_spec = md.time_spec.get_real_secs() + interval;
            }
            //send a mini EOB packet
            md.end_of_burst = true;
            tx_stream->send(Txbuffs, 0, md,1);
        }

        void rxFunc(
            uhd::usrp::multi_usrp::sptr usrp,
            const std::string &rx_cpu,
            uhd::rx_streamer::sptr rx_stream,
            double freq,
            double time,
            int numSamps,
            vector<wfm>* result
        ) 
        {
            uhd::set_thread_priority_safe();
            double interval = 10e-3; //10ms capture interval for multicaps

            vector<vector<complex<float>>> buffs; // a vector of units of the above type(multiple buff units)
            buffs.resize(result->size()); // size 1
            std::vector<complex<float> *> Rxbuff; // a vector of pointers to(arrays of) complex floats
            Rxbuff.reserve(result->size());
            for(int i = 0; i < buffs.size(); i++)
            {
                buffs[i].resize(numSamps);
                std::cout << buffs[i].size() << " samples requested." << std::endl;
                Rxbuff.push_back(buffs[i].data());
            }
            
            uhd::rx_metadata_t rxmd;
            uhd::time_spec_t theTime;
            theTime = usrp->get_time_now();
            uhd::stream_cmd_t
            cmd(uhd::stream_cmd_t::STREAM_MODE_NUM_SAMPS_AND_DONE);
            cmd.num_samps = numSamps;
            cmd.time_spec = time;
            cmd.stream_now = false;//dont stream now, use time spec

            if(time < theTime.get_real_secs())
            {
            std::cout << "timing problem!" << endl;
            std::cout <<"RXTIMESPEC "<< cmd.time_spec.get_real_secs() << endl;
            std::cout <<"USRP time(RxFunc) "<< theTime.get_real_secs()<<endl;
            std::cout <<"USRP Capture Time(RxFunc) "<< time<<endl;
            }

            int num_rx_samps = 0;
            int samps_per_buff = rx_stream->get_max_num_samps();
            int numSamplesReceived=0;
            int num_requested_samples = numSamps;
            std::vector<std::complex<double>> sampleBuffer(samps_per_buff);
            std::complex<double>* psampleBuffer = sampleBuffer.data();
            std::vector<double> fileBuffers(2*num_requested_samples);
            for(int i = 0; i < result->size();i++)
            {
                rx_stream->issue_stream_cmd(cmd);
                // TODO: implement max num samps from prev recv
                while (numSamplesReceived<num_requested_samples)
                {
                    double samplesForThisBlock=num_requested_samples-numSamplesReceived;
                    if (samplesForThisBlock>samps_per_buff){
                        samplesForThisBlock=samps_per_buff;
                    }
                    
                    size_t numNewSamples=rx_stream->recv(psampleBuffer,samplesForThisBlock,rxmd, 0.5);

                    double* pfileBuffers = fileBuffers.data() + numSamplesReceived*2; // destination to copy to
                    std::complex<double>* pSource = psampleBuffer; // received samples
                    std::memcpy(pfileBuffers, pSource, numNewSamples * sizeof(double) * 2);

                    //increment num samples receieved
                    numSamplesReceived+=numNewSamples;

                }

                cout <<"rx @ t= "<< rxmd.time_spec.get_real_secs()<< endl;
                cmd.time_spec = cmd.time_spec.get_real_secs()+interval;


                if(rxmd.error_code != 0)
                {
                    std::cout << "rxmd. "<< rxmd.time_spec.get_real_secs()<<std::endl;
                    std::cout << "Stored "<< num_rx_samps << " samples."<< std::endl;
                    std::cout << "Rx md error code: "<< rxmd.error_code << " RxMD errstr "<< rxmd.strerror()<< std::endl;
                }
            }

            // for now can assume result is size 1
            for(int i = 0; i < result->size();i++)
            {
                (*result)[i].resize(numSamps);  
                
                // ???
                if(usrp->get_master_clock_rate()>61.44e6)(*result)[i].Fs = 100e6;
                else (*result)[i].Fs = 30e6;


                for(int j = 0; j < numSamps; j ++) (*result)[i][j] = complex<double>(buffs[i][j].real(),buffs[i][j].imag());
            }


            if(rxmd.error_code != 0)
            {
                std::cout << "rxmd. "<< rxmd.time_spec.get_real_secs()<<std::endl;
                std::cout << "Stored "<< num_rx_samps << " samples."<< std::endl;
                std::cout << "Rx md error code: "<< rxmd.error_code << " RxMD errstr "<< rxmd.strerror()<< std::endl;
            }

            return;
        }

        void tx_rx_pulsed(uhd::usrp::multi_usrp::sptr usrp, config::usrp_config usrp_config)
        {
            std::string rx_cpu = "fc64";
            std::string tx_cpu = "fc64";
            ofstream *targetData;
            ofstream *targetVelocity;
            ofstream *log;
            wfm txWfm;
            txWfm.Fs = 25e6;
            double timeDuration = 1e-3;
            txWfm.samples.resize(txWfm.Fs *timeDuration);
            txWfm.generatePulseTrain(5,500,txWfm.samples.size()/500);// 50nS pulse duration, 5uS pulse period, 1mS total length
            uhd::set_thread_priority_safe();
            //setpriority(PRIO_PROCESS, 0, -20);

            double TxLOFreq = usrp_config.get_tx_rate();
            double RxLOFreq = TxLOFreq;
            int numSamps = 1e5;
            double T0 = 0.37;
            vector<wfm> singlePulsetrain;
            singlePulsetrain.resize(1);
            vector<wfm>singleCap;
            singleCap.resize(1);

            boost::thread_group thread_group;

            MTimer tickTock;
            tickTock.start();

            //create a receive streamer
            uhd::stream_args_t Rxstream_args("fc64","sc16");
            vector<size_t>rxChan;
            std::vector<size_t> channel_nums(1);
            rxChan.push_back(channel_nums[0]); // seg fault
            Rxstream_args.channels = rxChan;
            uhd::rx_streamer::sptr rx_stream = usrp->get_rx_stream(Rxstream_args);

            wfm dummyWfm = txWfm.PwrXcor(txWfm,-52,145); // delay the reference waveform from 520nS to 1450nS
            int imageDepth = 10;
            BMP radarDiagram;
            radarDiagram.SetSize(10*dummyWfm.size()+10,10*imageDepth+10);
            int radarDiagramWidth = 10*dummyWfm.size()+10;
            int radarDiagramHeight = 10*imageDepth+10;

            wfm origWfm = txWfm;
            if(usrp->get_master_clock_rate() < 61.44e6) txWfm.resample(3,10);
            wfm zerosWfm = txWfm;
            zerosWfm.clearIQ();
            txWfm.scaleMaxIQ(0.3);// to this to avoid overranging the DAC

            std::cout << "txSig numSamps after resample: "<< numSamps << std::endl;
            std::cout << "txWfm numSamps after resample: "<< txWfm.size() << std::endl;
            numSamps = txWfm.size();

            txWfm.saveENV("envs/transmittedWfm.env");
            singleCap[0] = txWfm;
            cout << "singleCap[0] sample rate: "<< singleCap[0].Fs << endl;

            ifstream runCheck;
            std::string runningPath =  "../radar-page/srv/httpd/htdocs/running";
            runCheck.open(runningPath.c_str());
            if (!runCheck.is_open()) 
            {
                std::cerr << "Failed to open file: " << runningPath << " - Error: " << strerror(errno) << std::endl;
            }

            ifstream killCheck;
            killCheck.open("../radar-page/srv/httpd/htdocs/killProc");
            deque<wfm>imgCaptures;

            imgCaptures.resize(10);
            for(int i =0 ; i < imgCaptures.size();i++)
            {
                imgCaptures[i] = zerosWfm;imgCaptures[i].resize(dummyWfm.size());
            }

            if(runCheck.good())cout << "All systems go."<< std::endl;
            double previousDistance = 1.0;
            double currentDistance = 1.0;
            //double currentSpeed = 0.0;
            double secondPeak = 0.0;
            double firstPeak = 0.0;
            double timeBetweenCaps = 0.0;
            double velocity = 0.0;

    
            while(!killCheck.good())
            {
            while(runCheck.good())
            {
                //create a transmit streamer
                uhd::stream_args_t Txstream_args("fc64","sc16");
                Txstream_args.channels = channel_nums;
                cout << "about to get stream args"<< endl;
                uhd::tx_streamer::sptr tx_stream = usrp->get_tx_stream(Txstream_args);
                cout << "about to reset usrp time"<< endl;
                usrp->set_time_now(uhd::time_spec_t(0.0)); // set t = 0
                cout<<"clocking timer"<< endl;
                tickTock.stop();
                timeBetweenCaps = tickTock.getDuration();
                std::cout << "Time since last run: "<< timeBetweenCaps <<endl;
                tickTock.start();
                tickTock.stop();
                cout << "Spawning RX Thread"<< endl;
                cout << "T = "<< tickTock.getDuration()<<endl;
                thread_group.create_thread(boost::bind(&rxFunc, usrp, rx_cpu, rx_stream, RxLOFreq, T0, numSamps, &singleCap));
                tickTock.stop();
                cout << "spawning Tx thread" << endl;
                thread_group.create_thread(boost::bind(&txFunc, usrp, tx_cpu, tx_stream, TxLOFreq, T0, numSamps, txWfm, zerosWfm,1));
                thread_group.join_all();
                tickTock.stop();
                cout<< "TX&RX threads done."<< endl;
                cout << "T = "<<tickTock.getDuration()<<endl;
                timeBetweenCaps += tickTock.getDuration();

                singleCap[0].saveENV("envs/BSeriesTestCapture.env"); // save rx cap to determine rx offset
                singleCap[0].resample(10,3);// peak search is done at 100msps for compatibility with better hardware
                singleCap[0].saveENV("envs/BSeriesTestCaptureUpsampled.env"); // save rx cap to determine rx offset
                imgCaptures.push_back(singleCap[0].PwrXcor(origWfm,-52,145));
                imgCaptures.back().resize((radarDiagramWidth/10)-1);
                imgCaptures.back().scaleMaxIQ(.99);
                cout << "xcored"<<endl;
                imgCaptures.pop_front();
                cout<<"popped back"<<endl;
                cout<<"imagesCaptures.size()="<< imgCaptures.size() << endl;
                cout<<"imagesCaptures[0].size()="<< imgCaptures[0].size() << endl;
                cout<<"maxwidth to be accessed: "<< 9+10*(imgCaptures[0].size()-1)+1<<endl;
                cout<<"maxheight to be accessed: "<< 9+10*(imgCaptures.size()-1)+1<<endl;
                
                /**
                for(int j = 0; j < imgCaptures.size();j++) // green bars
                {
                    for(int i = 0; i < imgCaptures[0].size();i++)
                    {
                        //int truncatedVal = generateHeatMapValue(imgCaptures[j][i].real());
                        for(int k = 0 ; k < 10 ; k++) for(int l = 0; l <10 ; l++)
                        {
                            int x = l + 10*i + 1;
                            int y = k + 10*j + 1;
                            if (x >= 0 && x < radarDiagramWidth && y >= 0 && y < radarDiagramHeight) 
                            {
                                radarDiagram.SetPixel(x, y, convertToRGBApixel(generateHeatMapValue(imgCaptures[j][i].real())));
                            }
                        }
                    }
                }

                for(int q = 0; q <radarDiagramWidth; q = q + 50)// blue ticks
                {
                    if(q+6 > radarDiagramWidth)break;
                    for(int k = 0; k < 3; k++)for(int j = 0; j < 10; j++)
                    {
                        int x = q + k + 3;
                        int y = radarDiagramHeight - 10 + j;
                        if (x >= 0 && x < radarDiagramWidth && y >= 0 && y < radarDiagramHeight)
                        {
                            //radarDiagram.SetPixel(x, y, convertToRGBApixel(255));
                        }
                    }
                }
                **/
                cout<< "checking if still active"<<endl;
                runCheck.close();
                runCheck.open("../radar-page/srv/httpd/htdocs/running");
                int minRange = 3;
                cout << "Min Range Setting(idx): "<< minRange <<endl;
                /**
                // erase old red tick
                for(int k = 0; k < 3; k++)for(int j = 0; j < 10; j++)
                radarDiagram.SetPixel(secondPeak*10+k+3,radarDiagramHeight-10+j,convertToRGBApixel(0x000000));
                cout <<"Peak Detection wfm Fs: "<< imgCaptures.back().Fs<<endl;
                imgCaptures.back().saveENV("envs/imgcapsback.env");
                firstPeak = imgCaptures.back().findMaxPowerIdx(0,minRange);
                secondPeak =
                imgCaptures.back().findHighestPowerPeak(minRange+firstPeak,imgCaptures.back().size()-1);
                cout << "firstpeak: "<< firstPeak << endl;
                cout << "secondPeak: "<< secondPeak << endl;

                //new red ticks
                for(int k = 0; k < 3; k++)for(int j = 0; j < 10; j++)
                radarDiagram.SetPixel(secondPeak*10+k+3,radarDiagramHeight-10+j,convertToRGBApixel(0xFF0000));
                currentDistance = (secondPeak - firstPeak)*3/2;
                velocity = (currentDistance-previousDistance)/(timeBetweenCaps/1000);// delta x over delta t (t is in milliseconds)
                cout << "uncalibrated distance(m): "<< currentDistance <<endl;
                cout << "velocity(m/s): "<< velocity << endl;
                (*targetData)<< currentDistance << endl;
                (*targetVelocity)<< velocity << endl;
                previousDistance = currentDistance;
                radarDiagram.WriteToFile("../radar-page/srv/httpd/htdocs/test.bmp");
                std::cout << "Diagram Written" << endl;
                cout<< "Rx Power: "<< singleCap[0].calcWfmPower()<<endl;
                */
            }

            killCheck.close();
            killCheck.open("../radar-page/srv/httpd/htdocs/stop");
            if(!killCheck.good()) 
            {    
                double durationInSeconds = 100e-3; // 0.1 seconds, or 100 milliseconds
                long long microseconds = static_cast<long long>(durationInSeconds * 1e6);
                boost::this_thread::sleep(boost::posix_time::microseconds(microseconds)); // wait
            }
            unlink("../radar-page/srv/httpd/htdocs/stop"); // for deleting

            //print summary
            std::cout << std::endl << boost::format(
            "Benchmark rate summary:\n"
            " Num received samples: %u\n"
            " Num dropped samples: %u\n"
            " Num overflows detected: %u\n"
            " Num transmitted samples: %u\n"
            " Num sequence errors: %u\n"
            " Num underflows detected: %u\n"
            ) % num_rx_samps % num_dropped_samps % num_overflows % num_tx_samps %
            num_seq_errors % num_underflows << std::endl;

            //finished
            log->close();
            targetData->close();
            delete(log);
            delete (targetData);
            std::cout << std::endl << "Done!" << std::endl << std::endl;

            }
        }



//-----------------------------------------------------------------------------------------------------------------------------
        void transmit_worker(uhd::usrp::multi_usrp::sptr tx_usrp, 
            std::vector<std::complex<double>> buffers, 
            double secondsInFuture, 
            uhd::tx_streamer::sptr tx_stream, 
            uhd::tx_metadata_t md)
        {
            hardware::transmitDoublesUntilStopped(tx_usrp, buffers, secondsInFuture, tx_stream, md);
        }

        void receive_worker(uhd::usrp::multi_usrp::sptr usrp,
            const std::string& file,
            int num_requested_samples,
            double settling_time,
            uhd::rx_streamer::sptr rx_stream,
            bool storeMD)
        {


            hardware::recv_to_file_doubles(usrp, rx_stream, file, num_requested_samples, settling_time, storeMD);
        }

        void pulsed2(uhd::usrp::multi_usrp::sptr usrp, config::usrp_config usrp_config)
        {
            // extract params from config
            std::string waveformFilename = usrp_config.get_waveform_filename();
            std::string outputFilename = "outputs/pulsed_test";
            double secondsInFuture = usrp_config.get_tx_start_time();
            double settlingTime = usrp_config.get_rx_start_time();
            double numRequestedSamples = usrp_config.get_num_samples();

            // Read in file 
            // TODO: assuming csv for now
            std::cout << "Reading in: " << waveformFilename << std::endl;


            std::vector<std::complex<double>> tx_buffers = utils::read_in_complex_csv(waveformFilename);


            //set up transmit streamer
            uhd::stream_args_t stream_args("fc64","sc16");
            std::vector<size_t> tx_channel_nums(0);
            stream_args.channels = tx_channel_nums;
            uhd::tx_streamer::sptr tx_stream= usrp->get_tx_stream(stream_args);

           // create a receive streamer - use same cpu and wire format
            std::vector<size_t> rx_channel_nums(0); 
            stream_args.channels             = rx_channel_nums;
            uhd::rx_streamer::sptr rx_stream = usrp->get_rx_stream(stream_args);
            
            uhd::tx_metadata_t md;
            md.has_time_spec=true;
            md.time_spec      =  uhd::time_spec_t(secondsInFuture);
            md.start_of_burst=true;

            // reset usrp time to prepare for transmit/receive
            std::cout << boost::format("Setting device timestamp to 0...") << std::endl;
            usrp->set_time_now(uhd::time_spec_t(0.0));


            // start tx thread with tx worker - this function creates the tx streamer
            // tx thread is waiting for ready call
            std::cout << "TX seconds in future: " << secondsInFuture << std::endl;
            std::cout << "RX settling time: " << settlingTime << std::endl;
            std::cout << "Writing results to: " << outputFilename << std::endl;
            boost::thread_group thread_group;
            thread_group.create_thread(boost::bind(&receive_worker, usrp, outputFilename, usrp_config.get_num_samples(),
            usrp_config.get_rx_start_time(), rx_stream, true));
            thread_group.create_thread(boost::bind(&transmit_worker, usrp, tx_buffers, usrp_config.get_tx_start_time(), tx_stream, md));

            // stop transmitting  
            //std::this_thread::sleep_for(std::chrono::microseconds((int)usrp_config.get_tx_start_time()*1000)); // have to wait at LEAST the amount of time we set commands to execute in the future
            std::this_thread::sleep_for(std::chrono::seconds(2));
            hardware::tx_stop_flag.store(true);

            thread_group.join_all();
        }
    }
}
