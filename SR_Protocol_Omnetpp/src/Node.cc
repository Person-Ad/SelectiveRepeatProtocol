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



Define_Module(Node);

void Node::initialize()
{
    CRCModule = new ErrorDetection("101");
    networkParams = NetworkParameters::loadFromModule(getParentModule());
    windowEnd = networkParams.WS - 1 ;
}

void Node::handleMessage(cMessage *msg)
{
     // Extract and validate the received message
    CustomMessage_Base *receivedMsg = check_and_cast<CustomMessage_Base *>(msg); 
    
    // Handle different message types based on frame type
    FrameType frameType = static_cast<FrameType>(receivedMsg->getFrameType());

    if (isCoordinatorInitiationMessage(receivedMsg->getFrameType())) {
        handleCoordinatorInitiation(receivedMsg);
    } else if (isSenderNode) {
         if (msg->isSelfMessage()){
             switch (frameType) {
                case FrameType::NACK:
                    // Handle NACK (Negative Acknowledgment)
                    EV << "Received NACK message" << std::endl;
                    // handleNack(receivedMsg);
                    break;

                case FrameType::ACK:
                    // Handle ACK (Acknowledgment)
                    EV << "Received ACK message" << std::endl;
                    // handleAck(receivedMsg);
                    break;

                case FrameType::SendTime:
                    // Send to Receiver
                    if(shouldContinueReading(windowStart,windowEnd,currentIndex)){
                        sendDataMessage(currentIndex);
                        incrementCircular(currentIndex);
                        scheduleNextMessage();
                    }
                    break;
                case FrameType::PrepareTime:
                    // Start Preparing 
                    scheduleNextMessage();
                    break;
                default:
                    // Handle unknown or unhandled frame types
                    EV << "Received unknown frame type" << std::endl;
                    break;
            }
      
         }else{
            handleAckNackResponse(receivedMsg);
         }
    } else {
        handleIncomingDataMessage(receivedMsg);
    }

}
bool Node::isCoordinatorInitiationMessage(int frameType) 
{
    return static_cast<FrameType>(frameType) == FrameType::Control; // Coordinator start signal
}

void Node::handleCoordinatorInitiation(CustomMessage_Base *receivedMsg) 
{
    // Set sender node flag
    isSenderNode = true;  
    
    // Determine node index from name
    int nodeIndex = extractNodeIndex();
    
    // Read input file for this node
    lines = Utils::readFileLines(generateInputFilePath(nodeIndex)); 
    
    // Prepare and send first message
    int startTime = atoi(receivedMsg->getPayload());
    CustomMessage_Base *customMessage = new CustomMessage_Base();
    customMessage->setFrameType(static_cast<int>(FrameType::PrepareTime));
    scheduleAt(simTime() + startTime, customMessage);
    Logger::initialize("../text_files/output.txt",nodeIndex);
}

int Node::extractNodeIndex() 
{
    std::string nodeName = std::string(getName()); 
    return nodeName.back() - '0'; 
}

void Node::incrementCircular(int & number){
    number = (number + 1)%networkParams.WS;
}
void Node::incrementWindowCircular(int & number){
    number = (number + 1)%(networkParams.WS+1);
}
std::string Node::generateInputFilePath(int nodeIndex) 
{
    return "../text_files/input" + std::to_string(nodeIndex) + ".txt";
}

void Node::handleAckNackResponse(CustomMessage_Base *receivedMsg) 
{
    // Implement ACK/NACK handling logic
    // This method is currently empty in the original code
    // Add appropriate logic for handling sender node responses
    if (static_cast<FrameType>(receivedMsg->getFrameType()) == FrameType::ACK) {
        handleAckResponse(receivedMsg);
    }else if (static_cast<FrameType>(receivedMsg->getFrameType()) == FrameType::NACK) {
        handleNackResponse(receivedMsg);
    }

}
void Node::handleAckResponse(CustomMessage_Base *receivedMsg)
{
    incrementWindowCircular(windowStart);
    incrementWindowCircular(windowEnd);
}
void Node::handleNackResponse(CustomMessage_Base *receivedMsg)
{

}

void Node::handleIncomingDataMessage(CustomMessage_Base *receivedMsg) 
{
    // Extract message details
    std::string payload = receivedMsg->getPayload(); 
    char trailerChar = receivedMsg->getTrailer(); 
    std::string trailer = std::bitset<8>(trailerChar).to_string(); 
 
    // Validate CRC
    bool valid = validateMessageCRC(payload, trailer);
    
    if (!valid) {
        handleCRCError();
    } else {
        processValidReceivedMessage(payload);
    }
}

bool Node::validateMessageCRC(const std::string& payload, const std::string& trailer) 
{
    return CRCModule->validateCRC(Utils::stringToBinaryStream(payload) + trailer);
}

void Node::handleCRCError() 
{
    std::cout << "Error occurred\n";
}

void Node::processValidReceivedMessage(const std::string& payload) 
{
     std::string unstuffedMessage = Framing::unstuff(payload);
     Logger::logUpload(simTime().dbl(), unstuffedMessage, receiverCurrentIndex);

}

void Node::sendDataMessage(int index){
    // Extract first message from lines
    std::pair<int,std::string> extractedMessage = Utils::extractMessage(lines[index]); 
    int errorNumber = extractedMessage.first;  
    std::string message = extractedMessage.second;  
    
    // Stuff the message and compute CRC
    std::string stuffedMessage = Framing::stuff(message); 
    std::string CRC = CRCModule->computeCRC(Utils::stringToBinaryStream(stuffedMessage)); 
 
    // Send the data message
    CustomMessage_Base *customMessage = new CustomMessage_Base();
    customMessage->setPayload(stuffedMessage.c_str());
    customMessage->setName(customMessage->getPayload());
    char trailerChar = static_cast<char>(std::stoi(CRC, nullptr, 2));
    customMessage->setTrailer(trailerChar);
    sendDelayed(customMessage, networkParams.TD ,"dataGate$o");
    Logger::logChannelError(simTime().dbl(),Utils::toBinary4Bits(errorNumber));
    Logger::logFrameSent(simTime().dbl(), index, stuffedMessage, CRC, false, false, false, 0);


}

bool Node::shouldContinueReading(int rangeStart, int rangeEnd, int currentSeq) {
    // Case 1: Non-wrapping range [rangeStart, rangeEnd)
    if (rangeStart <= rangeEnd) {
        return currentSeq >= rangeStart && currentSeq < rangeEnd;
    }
    
    // Case 2: Wrapping range, e.g., [rangeStart, rangeEnd) in circular buffers
    return (currentSeq >= rangeStart || currentSeq < rangeEnd);
}

void Node::scheduleNextMessage() {
    CustomMessage_Base* nextMessage = new CustomMessage_Base();

    // Set the frame type to 'SendTime'
    nextMessage->setFrameType(static_cast<int>(FrameType::SendTime));

    // Schedule the next message after the specified PT (Propagation Time)
    scheduleAt(simTime() + networkParams.PT, nextMessage);
}