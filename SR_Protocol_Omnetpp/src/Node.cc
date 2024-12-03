//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "Node.h"

#include "CustomMessage_m.h"
#include "Utils.h"
#include "Framing.h"
#include "Logger.h"
#include <bitset> 

Define_Module(Node);

void Node::initialize()
{
    CRCModule = new ErrorDetection("101");
}

void Node::handleMessage(cMessage *msg)
{
    CustomMessage_Base *receivedMsg = check_and_cast<CustomMessage_Base *>(msg);
    std::string payload = receivedMsg->getPayload();
    char trailerChar = receivedMsg->getTrailer();
    std::string trailer = std::bitset<8> (trailerChar).to_string();

    int frameType = receivedMsg->getFrameType();
    if (frameType == 3){ // It means it's from coordinator and time to start
        isSenderNode = true; 
        std::string nodeName = std::string(getName());
        int nodeIndex = nodeName.back() - '0';
        // Reading the file and storing it 
        lines = Utils::readFileLines("../text_files/input" + std::to_string(nodeIndex) + ".txt");
        
        // Sending the first message 
        std::pair<int,std::string> extractedMessage = Utils::extractMessage(lines[0]);
        int errorNumber = extractedMessage.first; 
        std::string message = extractedMessage.second; 
        std::string stuffedMessage = Framing::stuff(message);
        std::string CRC = CRCModule->computeCRC(Utils::stringToBinaryStream(stuffedMessage));

        sendDataMessage(stuffedMessage, CRC);
    }else if(isSenderNode){ // It means I will receive ACK/NACK 
        
    }else{ // It means I received data
        // Validating CRC 
        bool valid = CRCModule->validateCRC(Utils::stringToBinaryStream(payload) + trailer);
        if(!valid){
            std::cout<<"Error occurred\n";
        }else{
            std::cout<<"Valid !\n";
            std::string unstuffedMessage = Framing::unstuff(payload);
            EV << unstuffedMessage <<"\n";
        }
    }
    

}

void Node::sendDataMessage(std::string messageValue, std::string trailer){
    CustomMessage_Base *customMessage = new CustomMessage_Base();
    customMessage->setPayload(messageValue.c_str());
    customMessage->setName(customMessage->getPayload());
    char trailerChar = static_cast<char>(std::stoi(trailer, nullptr, 2));
    customMessage->setTrailer(trailerChar);
    send(customMessage, "dataGate$o");
}
