/*
 * Copyright (C) 2018  Christian Berger
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cluon-complete.hpp"
#include "opendlv-standard-message-set.hpp"

#include <cstdint>
#include <iostream>
#include <string>
#include <thread>

void sendMessage(cluon::OD4Session &od4, std::string text){
    std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();
    cluon::data::TimeStamp sampleTime = cluon::time::convert(tp);
    uint32_t senderStamp=0;

    opendlv::system::SignalStatusMessage msg;
    msg.description(text);

    od4.send(msg, sampleTime, senderStamp); 
}


void callOnReceive(cluon::data::Envelope data){
    if (data.dataType() == static_cast<int32_t>(opendlv::system::NetworkStatusMessage::ID())) {
        opendlv::system::NetworkStatusMessage t = cluon::extractMessage<opendlv::system::NetworkStatusMessage>(std::move(data));
        std::cout << "Test function: NetworkStatusMessage data:" << t.description() << std::endl;
        
        int CID = 112;
        cluon::OD4Session od4s1{static_cast<uint16_t>(CID),
            [](auto){}
        };
        sendMessage(od4s1, t.description() + "456");

    }
}


int32_t main(int32_t argc, char **argv) {

    int32_t retCode{0};
    auto commandlineArguments = cluon::getCommandlineArguments(argc, argv);
    if ( 0 == commandlineArguments.count("cid"))  {
        std::cerr << argv[0] << " testing unit and publishes it to a running OpenDaVINCI session using the OpenDLV Standard Message Set." << std::endl;
        std::cerr << "Usage:   " << argv[0] << "--cid=<OpenDaVINCI session>" << std::endl;
        std::cerr << "Example: " << argv[0] << "--cid=111" << std::endl;
        retCode = 1;
    } else {

    
    // Interface to a running OpenDaVINCI session.
    cluon::data::Envelope data;

    cluon::OD4Session od4{static_cast<uint16_t>(std::stoi(commandlineArguments["cid"])),
        [&data](cluon::data::Envelope &&envelope){
            callOnReceive(envelope);
            // IMPORTANT INTRODUCE A MUTEX
            data = envelope;
        }
    };

    // Just sleep as this microservice is data driven.
    using namespace std::literals::chrono_literals;

    int count = 0;
    while (od4.isRunning()) {
        std::this_thread::sleep_for(1s);
        count++;
        std::cout << count << std::endl;
    }
}
     return retCode;


}