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
#include <queue> 

using namespace omnetpp;

/**
 * TODO - Generated class
 */
struct Frame {
    bool isLoss = false;           // Indicates whether the frame is lost
    int duplicate = 0;             // 0 for none, 1 for first version, 2 for second version
    int delay = 0;                 // 0 for no delay, otherwise the delay interval
    int modificationBit = -1;      // -1 for no modification, otherwise the modified bit number

    CustomMessage_Base* message = nullptr; // Pointer to the actual message
};

struct NetworkParameters {
    int WS;    // Window Size
    int SN;    // Seq Num
    double TO; // Timeout
    double PT; // Processing Time
    double TD; // Transmission Delay
    double ED; // End Delay
    double DD; // Duplication Delay
    double LP; // Loss Probability

    // Factory method to load parameters
    static NetworkParameters loadFromModule(cModule* parentModule) {
        NetworkParameters params;
        
        params.WS = parentModule->par("WS").intValue();
        params.SN = parentModule->par("SN").intValue();
        params.TO = parentModule->par("TO").doubleValue();
        params.PT = parentModule->par("PT").doubleValue();
        params.TD = parentModule->par("TD").doubleValue();
        params.ED = parentModule->par("ED").doubleValue();
        params.DD = parentModule->par("DD").doubleValue();
        params.LP = parentModule->par("LP").doubleValue();

        return params;
    }
};

enum class FrameType {
    NACK = 0, // Negative Acknowledgment
    ACK = 1,  // Acknowledgment
    Data = 2,  // Data Frame
    Control = 3, // Control Frame from Coordinator
    SendTime = 4, // Means it's time to send schedule Processing Time finished
    PrepareTime=5, // Means it's time to process 
    DuplicatedFrame = 6,
    Timeout = 7,
};

class Node : public cSimpleModule
{
  public:
    NetworkParameters networkParams;
    bool isSenderNode = false; 
    // Sender related parameters
    std::queue<std::string> packets;
    int windowStart = 0;      // Start index of the window
    int windowEnd = 0;        // End index of the window
    int currentIndex = 0;     // Current index being processed within the window
    int nbuffered = 0; // Number of packets buffered 
    int ack_expected = 0;
    std::vector<Frame *> buffer; 
    // CRC Error Detection 
    ErrorDetection * CRCModule ;
    // Receiver related parameters
    int receiverWindowStart = 0;      // Start index of the window
    int receiverWindowEnd = 0;        // End index of the window
    int receiverCurrentIndex = 0;     // Current index being processed within the window
    int frame_expected = 0;
    int too_far = 0; 
    std::vector<Frame *> in_buffer;

    bool isProcessing = false;
    
    bool isModified = false;
    bool isLoss = false;
    bool isDuplicate = false;
    bool isDelayed = false;
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    void sendDataMessage(int);


    // Message type detection methods
    bool isCoordinatorInitiationMessage(int frameType);

    // Message handling methods
    void handleCoordinatorInitiation(CustomMessage_Base *receivedMsg);
    void handleAckNackResponse(CustomMessage_Base *receivedMsg);
    void handleAckResponse(CustomMessage_Base *receivedMsg);
    void handleNackResponse(CustomMessage_Base *receivedMsg);
    void handleIncomingDataMessage(CustomMessage_Base *receivedMsg);
    bool shouldContinueReading(int rangeStart, int rangeEnd, int currentSeq);
    void processMessage(int);

    // Network Layer 
    void to_network_layer(CustomMessage_Base *receivedMsg);
    // Timer Methods 
    void startTimer(CustomMessage_Base *msgToSend ,  int index);
    void stopTimer(int);
    // Utility methods for message processing
    int extractNodeIndex();
    std::string generateInputFilePath(int nodeIndex);
    
    // CRC and message validation methods
    bool validateMessageCRC(const std::string& payload, const std::string& trailer);
    void handleCRCError(CustomMessage_Base* receivedMsg);
    void processValidReceivedMessage(CustomMessage_Base *receivedMsg);

    // Window Functions 
    void incrementCircular(int & number);
    void incrementWindowCircular(int & number);

    // Message Functions 
    Frame * parseFlags(const std::string& errorNumber, const std::string message);
    // Logging 

    // Errors 
    std::string modifyMessage(const std::string& message, int errorBit);

    // Timeout 
    std::vector<CustomMessage_Base *> timeoutMessages;
    void handleTimeout(CustomMessage_Base *msg);
};

#endif
