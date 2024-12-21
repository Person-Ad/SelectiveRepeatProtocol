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
    CRCModule = new ErrorDetection("111");
    networkParams = NetworkParameters::loadFromModule(getParentModule());
    windowEnd = networkParams.WS - 1 ;
    buffer = std::vector<Frame *>(networkParams.WS,nullptr);
    in_buffer = std::vector<Frame *>(networkParams.WS,nullptr);
    timeoutMessages = std::vector<CustomMessage_Base *>(networkParams.WS,nullptr);
    too_far = networkParams.WS ;
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
        switch (frameType) {
            case FrameType::NACK:
                // Handle NACK (Negative Acknowledgment)
                handleNackResponse(receivedMsg);
                break;

            case FrameType::ACK:
                // Handle ACK (Acknowledgment)
                handleAckResponse(receivedMsg);
                break;

            case FrameType::SendTime:
                // Send to Receiver
                sendDataMessage(currentIndex);
                incrementCircular(currentIndex);
                processMessage(currentIndex);
                break;
            case FrameType::PrepareTime:
                // Start Preparing 
                processMessage(currentIndex);
                break;
            case FrameType::DuplicatedFrame:
                // Start Preparing 
                // sendDuplicateMessage(receivedMsg);
                break;
            case FrameType::Timeout:
                handleTimeout(receivedMsg);
                break;
            case FrameType::SendDataTimeout:
                sendTimeoutDataMessage(receivedMsg);
                break;
            default:
                // Handle unknown or unhandled frame types
                EV << "Received unknown frame type" << std::endl;
                break;
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
    std::vector<std::string> lines = Utils::readFileLines(generateInputFilePath(nodeIndex)); 
    for (const auto& line : lines) {
        packets.push(line);
    }
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
    number = (number + 1)%(networkParams.SN+1);
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
    int ackNo = receivedMsg->getAckNackNumber();
    if (ackNo >= ack_expected && ackNo < currentIndex) {
        while (ack_expected != ackNo) {
            stopTimer(ack_expected); // Stop timers for acknowledged frames
            nbuffered--;
            ack_expected = (ack_expected + 1) % (networkParams.SN + 1); // Slide window
        }
    }
    processMessage(currentIndex);
}
void Node::handleNackResponse(CustomMessage_Base *receivedMsg)
{
    //TODO: Add here logic to stop the processing packet by adding a boolean after the PT to schedule another PT and resend the corrupted packet
    stopTimer(receivedMsg->getHeader());
}

void Node::handleIncomingDataMessage(CustomMessage_Base *receivedMsg) 
{
    //TODO: Add Handling Lost ACK 
    double random = uniform(0, 1);
    bool lostACK = random < networkParams.LP ? true : false;
    // Extract message details
    std::string payload = receivedMsg->getPayload(); 
    char trailerChar = receivedMsg->getTrailer(); 
    std::string trailer = std::bitset<8>(trailerChar).to_string(); 
 
    // Validate CRC
    bool valid = validateMessageCRC(payload, trailer);
    
    if (!valid) {
        handleCRCError(receivedMsg);
    } else {
        processValidReceivedMessage(receivedMsg);
    }
}

bool Node::validateMessageCRC(const std::string& payload, const std::string& trailer) 
{
    return CRCModule->validateCRC(Utils::stringToBinaryStream(payload) + trailer);
}

void Node::handleCRCError(CustomMessage_Base* receivedMsg) 
{   
    // Send NACK only if in order
    if(receivedMsg->getHeader() == frame_expected){
        CustomMessage_Base* nackMessage = new CustomMessage_Base();
        nackMessage->setFrameType(static_cast<int>(FrameType::NACK));
        sendDelayed(nackMessage, networkParams.TD ,"dataGate$o");
        Logger::logACK(simTime().dbl(), frame_expected , false, false);
    }
}

void Node::processValidReceivedMessage(CustomMessage_Base* receivedMsg) 
{
    Frame * frame = new Frame();
    frame->message = receivedMsg;
    in_buffer[receivedMsg->getHeader() % networkParams.WS] = frame;
     // Send ACK to sender 
    while (in_buffer[frame_expected % networkParams.WS] != nullptr) {
        to_network_layer(in_buffer[frame_expected % networkParams.WS]->message);
        in_buffer[frame_expected % networkParams.WS] = nullptr;
        frame_expected = (frame_expected + 1) % (networkParams.SN + 1);
        too_far = (too_far + 1) % (networkParams.SN + 1);
    }
     CustomMessage_Base* ackMessage = new CustomMessage_Base();
     int ackNo = frame_expected;
     ackMessage->setAckNackNumber(ackNo);
     ackMessage->setFrameType(static_cast<int>(FrameType::ACK));
     sendDelayed(ackMessage, networkParams.TD ,"dataGate$o");
     Logger::logACK(simTime().dbl(), ackNo, true, false);
}

void Node::sendDataMessage(int index){
    isProcessing = false;
    // Extract relevant data from the message
    Frame * frame = buffer[index];
    CustomMessage_Base* msgToSend = frame->message;
    std::string stuffedMessage = msgToSend->getPayload();
    std::string CRC = Utils::binaryStringFromChar(msgToSend->getTrailer());
    
    // Modify message if needed 
    std::string modifiedMessage = modifyMessage(stuffedMessage, frame->modificationBit);
    msgToSend ->setPayload(modifiedMessage.c_str());
    msgToSend->setFrameType(static_cast<int>(FrameType::Data));

    // Send the data message
    Logger::logFrameSent(simTime().dbl(), index, modifiedMessage, CRC, frame->modificationBit , frame->isLoss, frame->duplicate, frame->delay);
    // Only send if Loss didn't occur 
    if(!frame->isLoss){
        sendDelayed(msgToSend, networkParams.TD ,"dataGate$o");
    }
    // Check Duplicate 
    if(frame->duplicate){
        CustomMessage_Base* duplicatedMsg = msgToSend->dup();
        sendDelayed(duplicatedMsg, networkParams.TD + networkParams.DD ,"dataGate$o");
        Logger::logFrameSent(simTime().dbl() + networkParams.DD, index, stuffedMessage, CRC, frame->modificationBit , frame->isLoss, 2, frame->delay);
    }

    // Add Timeout 
    CustomMessage_Base* timeoutMsg = msgToSend->dup();
    // Add the unmodified payload to the timeout message
    timeoutMsg->setPayload(stuffedMessage.c_str());
    startTimer(timeoutMsg, index);

}

void Node::sendTimeoutDataMessage(CustomMessage_Base *msg){
    isProcessing = false;
    // Extract relevant data from the message
    std::string stuffedMessage = msg->getPayload();
    std::string CRC = Utils::binaryStringFromChar(msg->getTrailer());
    
    msg ->setPayload(stuffedMessage.c_str());
    msg->setFrameType(static_cast<int>(FrameType::Data));
    
    // Send the data message
    sendDelayed(msg, networkParams.TD ,"dataGate$o");
    Logger::logFrameSent(simTime().dbl(), msg->getHeader(), stuffedMessage, CRC, -1 , false , 0, 0);

}

bool Node::shouldContinueReading(int rangeStart, int rangeEnd, int currentSeq) {
    // Log the input values
    EV << "shouldContinueReading called with rangeStart=" << rangeStart 
       << ", rangeEnd=" << rangeEnd 
       << ", currentSeq=" << currentSeq << endl;

    bool result;
    
    // Case 1: Non-wrapping range [rangeStart, rangeEnd)
    if (rangeStart <= rangeEnd) {
        result = (currentSeq >= rangeStart && currentSeq < rangeEnd);
    }
    // Case 2: Wrapping range, e.g., [rangeStart, rangeEnd) in circular buffers
    else {
        result = (currentSeq >= rangeStart || currentSeq < rangeEnd);
    }

    // Log the result before returning
    EV << "Result of shouldContinueReading: " << (result ? "true" : "false") << endl;

    return result;
}


void Node::processMessage(int index) {
    if(isProcessing){
        return;
    }
    isProcessing = true;
    // It means I can't process more because the buffer is full or there is no more packets
    if(nbuffered >= networkParams.WS || packets.empty()){
        return ; 
    }
    CustomMessage_Base* customMessage = new CustomMessage_Base();
    
    // Extract first message from lines
    std::string nextLine = packets.front();
    packets.pop();
    std::pair<std::string,std::string> extractedMessage = Utils::extractMessage(nextLine); 
    std::string errorNumber = extractedMessage.first;  
    std::string message = extractedMessage.second;  
    // Parse Errors 

    // Stuff the message and compute CRC
    std::string stuffedMessage = Framing::stuff(message); 
    std::string CRC = CRCModule->computeCRC(Utils::stringToBinaryStream(stuffedMessage)); 
    
    Frame * frame = parseFlags(errorNumber,stuffedMessage);
    
    char trailerChar = static_cast<char>(std::stoi(CRC, nullptr, 2));
    customMessage->setTrailer(trailerChar);
    customMessage->setHeader(index);
    
    // Here I didn't modify for the reason of timeout to have a clean and modified version at sendData
    customMessage->setPayload(stuffedMessage.c_str());
    customMessage->setName(customMessage->getPayload());

    // Set the frame type to 'SendTime'
    customMessage->setFrameType(static_cast<int>(FrameType::SendTime));
    // Schedule the next message after the specified PT (Process Time)
    scheduleAt(simTime() + networkParams.PT, customMessage);

    // Adding the frame to buffer 
    frame->message = customMessage; 
    buffer[index % (networkParams.WS)] = frame;
    // Log 
    Logger::logChannelError(simTime().dbl(),errorNumber);
}

void Node::startTimer(CustomMessage_Base *msgToSend ,  int index){
    timeoutMessages[index] = msgToSend->dup();
    timeoutMessages[index]->setFrameType(static_cast<int>(FrameType::Timeout));
    scheduleAt(simTime() + networkParams.TO , timeoutMessages[index]);
}

void Node::stopTimer(int index){
    // Clear Timeouts 
    if (timeoutMessages[index] && timeoutMessages[index]->isScheduled()) // check if timer is scheduled
    {
        cancelAndDelete(timeoutMessages[index]); // delete the timer message
        timeoutMessages[index] = nullptr;
    }
}

void Node::handleTimeout(CustomMessage_Base *msg){

    Logger::logTimeout(simTime().dbl(),  msg->getHeader());
    //TODO: Add ScheduleAt with PT to send the Unmodified message
    msg->setFrameType(static_cast<int>(FrameType::SendDataTimeout));
    scheduleAt(simTime() + networkParams.PT, msg);
}

void Node::to_network_layer(CustomMessage_Base *receivedMsg){
    std::string unstuffedMessage = Framing::unstuff(receivedMsg->getPayload());
    Logger::logUpload(simTime().dbl(), unstuffedMessage, receiverCurrentIndex);
}

Frame * Node::parseFlags(const std::string& errorNumber, const std::string message) {
    if (errorNumber.size() != 4) {
        throw std::invalid_argument("Invalid errorNumber length: " + errorNumber);
    }
    Frame* newFrame = new Frame();

    // Parse the errorNumber and set the frame attributes
    int randomBit = int(uniform(0, message.size() * 8));
    newFrame->modificationBit = (errorNumber[0] == '1') ? randomBit : -1; // Assuming bit 0 is modified
    newFrame->isLoss = (errorNumber[1] == '1');
    newFrame->duplicate = (errorNumber[2] == '1') ? 1 : 0;      
    newFrame->delay = (errorNumber[3] == '1') ? networkParams.DD : 0;           

    return newFrame;
}

std::string Node::modifyMessage(const std::string& message, int errorBit) {
    if(errorBit < 0){
        return message; 
    }
    if (message.empty()) {
        throw std::invalid_argument("Message cannot be empty.");
    }

    int messageLength = message.size();
    std::vector<std::bitset<8>> messageBitStream(messageLength);

    // Convert each character to its bit representation
    for (int i = 0; i < messageLength; ++i) {
        messageBitStream[i] = std::bitset<8>(message[i]);
    }

    int errorChar = errorBit / 8;
    int errorBitPos = errorBit % 8;

    // Toggle the chosen bit
    messageBitStream[errorChar][errorBitPos].flip();

    // Reconstruct the message from the modified bit stream
    std::string erroredMessage;
    erroredMessage.reserve(messageLength); // Reserve memory to avoid reallocations
    for (const auto& bits : messageBitStream) {
        erroredMessage.push_back(static_cast<char>(bits.to_ulong()));
    }

    return erroredMessage;
}
