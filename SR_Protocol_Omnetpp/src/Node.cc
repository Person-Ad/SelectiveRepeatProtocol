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
}

void Node::handleMessage(cMessage *msg)
{
     // Extract and validate the received message
    CustomMessage_Base *receivedMsg = check_and_cast<CustomMessage_Base *>(msg); 
    
    // Handle different message types based on frame type
    int frameType = receivedMsg->getFrameType(); 
    
    if (isCoordinatorInitiationMessage(frameType)) {
        handleCoordinatorInitiation(receivedMsg);
    } else if (isSenderNode) {
         if (msg->isSelfMessage()){
                sendDataMessage(send_next_frame);
                incrementCircular(send_next_frame);
         }else{
            handleAckNackResponse(receivedMsg);
         }
    } else {
        handleIncomingDataMessage(receivedMsg);
    }

}
bool Node::isCoordinatorInitiationMessage(int frameType) 
{
    return frameType == 3; // Coordinator start signal
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
    scheduleAt(simTime() + startTime, customMessage);
    logger = new Logger("../text_files/output.txt",nodeIndex);
}

int Node::extractNodeIndex() 
{
    std::string nodeName = std::string(getName()); 
    return nodeName.back() - '0'; 
}

void Node::incrementCircular(int & number){
    number = (number + 1)%networkParams.WS;
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
        processValidMessage(payload);
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

void Node::processValidMessage(const std::string& payload) 
{
    std::cout << "Valid !\n";
    std::string unstuffedMessage = Framing::unstuff(payload); 
    EV << unstuffedMessage << "\n";
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
    send(customMessage, "dataGate$o");
    logger->logFrameSent(simTime().dbl(), index, stuffedMessage, CRC, false, false, false, 0);
}
