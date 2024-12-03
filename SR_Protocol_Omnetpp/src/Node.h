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

#ifndef __SR_PROTOCOL_OMNETPP_NODE_H_
#define __SR_PROTOCOL_OMNETPP_NODE_H_

#include <omnetpp.h>
#include "ErrorDetection.h"
#include "CustomMessage_m.h"
#include "Utils.h"
#include "Framing.h"
#include "Logger.h"
#include <bitset> 

using namespace omnetpp;

/**
 * TODO - Generated class
 */
class Node : public cSimpleModule
{
  public:
    int WS = 0;
    double TO = 0;
    double PT = 0;
    double TD = 0;
    double ED = 0;
    double DD = 0;
    double LP = 0;

    bool isSenderNode = false; 
    
    // Sender related parameters
    std::vector<std::string> lines;
    int send_next_frame = 0; 

    // CRC Error Detection 
    ErrorDetection * CRCModule ;

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    void sendDataMessage(int index);


    // Message type detection methods
    bool isCoordinatorInitiationMessage(int frameType);

    // Message handling methods
    void handleCoordinatorInitiation(CustomMessage_Base *receivedMsg);
    void handleAckNackResponse(CustomMessage_Base *receivedMsg);
    void handleIncomingDataMessage(CustomMessage_Base *receivedMsg);

    // Utility methods for message processing
    int extractNodeIndex();
    std::string generateInputFilePath(int nodeIndex);
    
    // CRC and message validation methods
    bool validateMessageCRC(const std::string& payload, const std::string& trailer);
    void handleCRCError();
    void processValidMessage(const std::string& payload);

};

#endif
