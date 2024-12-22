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
std::string x = "0";

void Node::initialize()
{
    CRCModule = new ErrorDetection("111");
    networkParams = NetworkParameters::loadFromModule(getParentModule());
    windowEnd = networkParams.WS - 1 ;
    buffer = std::vector<Frame *>(networkParams.WS,nullptr);
    in_buffer = std::vector<Frame *>(networkParams.WS,nullptr);
    sentNack = std::vector<bool>(networkParams.WS,false);
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
                if(interruptTimeoutNack){
                    EV<<"TimeoutNACK Active: "<<timeoutNackFinishTime<<"\n";
                    scheduleAt(timeoutNackFinishTime + networkParams.PT, receivedMsg);
                    interruptTimeoutNack = false;
                }else{
                    sendNextDataMessage();
                }
                break;
            case FrameType::PrepareTime:
                // Start Preparing 
                processMessage(next_frame_to_send);
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
        switch (frameType) {
            case FrameType::NACK:
                // Handle NACK (Negative Acknowledgment)
                sendNackMessage(receivedMsg);
                break;
            case FrameType::ACK:
                // Handle ACK (Acknowledgment)
                sendAckMessage(receivedMsg);
                break;
            default:
                // Handle it as Data
                handleIncomingDataMessage(receivedMsg);
                break;
        }
       
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
        packets.push_back(line);
    }
    // Prepare and send first message
    int startTime = atoi(receivedMsg->getPayload());
    CustomMessage_Base *customMessage = new CustomMessage_Base();
    customMessage->setFrameType(static_cast<int>(FrameType::PrepareTime));
    scheduleAt(simTime() + startTime, customMessage);
    Logger::initialize("../text_files/output" + x + ".txt",nodeIndex);
}

int Node::extractNodeIndex() 
{
    std::string nodeName = std::string(getName()); 
    return nodeName.back() - '0'; 
}

void Node::incrementCircular(int & number){
   number = (number + 1)%(networkParams.SN+1);
}

std::string Node::generateInputFilePath(int nodeIndex) 
{
    nodeIndex = 7; 
    // return "../text_files/input" + std::to_string(nodeIndex) + ".txt";
    return "../text_files/input" + x + ".txt";

}

void Node::handleAckResponse(CustomMessage_Base *receivedMsg)
{
    // isProcessing = false;
    int ackNo = receivedMsg->getAckNackNumber();
    EV << "Received ACK: " << ackNo << ", Expected ACK: " << ack_expected
    << ", Next Frame to Send: " << next_frame_to_send << "\n";
    // Circular check for ACK within the sliding window
    if (Utils::between(ack_expected, ackNo, next_frame_to_send)) {
        while (ack_expected != ackNo) {
            EV <<"Stoping Timer Index: "<<ack_expected<<"\n";
            stopTimer(ack_expected % networkParams.WS); // Stop timers for acknowledged frames
            nbuffered--;
            incrementCircular(ack_expected); // Slide window
        }
    }
    processMessage(next_frame_to_send);
}
void Node::handleNackResponse(CustomMessage_Base *receivedMsg)
{
    // isProcessing = false;
    //TODO: Add here logic to stop the processing packet by adding a boolean after the PT to schedule another PT and resend the corrupted packet
    int seqNo = static_cast<int>(receivedMsg->getAckNackNumber());
    EV << "Received NACK: " << seqNo << "\n";
    // stopTimer(seqNo % networkParams.WS);
    Frame * frame = buffer[seqNo % (networkParams.WS)];
    CustomMessage_Base * cleanMessage = frame->message->dup(); 
    cleanMessage->setFrameType(static_cast<int>(FrameType::SendDataTimeout));
    scheduleAt(simTime() + networkParams.PT + 0.001, cleanMessage);
    interruptTimeoutNack = true; 
    timeoutNackFinishTime = simTime().dbl() + networkParams.PT + 0.001;
}

void Node::sendNextDataMessage(){
    // This means that I can't send now 
    isProcessing = false;
    // Extract relevant data from the message
    Frame * frame = buffer[next_frame_to_send % (networkParams.WS)];
    CustomMessage_Base* msgToSend = frame->message->dup();
    std::string stuffedMessage = msgToSend->getPayload();
    std::string CRC = Utils::binaryStringFromChar(msgToSend->getTrailer());
    
    // Modify message if needed 
    std::string modifiedMessage = modifyMessage(stuffedMessage, frame->modificationBit);
    msgToSend ->setPayload(modifiedMessage.c_str());
    msgToSend->setFrameType(static_cast<int>(FrameType::Data));

    EV << "Sending Data Message next_frame_to_send : " << next_frame_to_send<< " Modified Message : "<<modifiedMessage<<"\n";
    // Send the data message
    Logger::logFrameSent(simTime().dbl(), next_frame_to_send, modifiedMessage, CRC, frame->modificationBit , frame->isLoss, frame->duplicate, frame->delay);
    // Only send if Loss didn't occur 
    if(!frame->isLoss){
        // Sending the frame and adding delay if exists 
        sendDelayed(msgToSend, networkParams.TD + frame->delay ,"dataGate$o");
    }
    // Check Duplicate 
    if(frame->duplicate){
        CustomMessage_Base* duplicatedMsg = msgToSend->dup();
        // The duplicate message has the same problems 
        if(!frame->isLoss){
            sendDelayed(duplicatedMsg, networkParams.TD + networkParams.DD + frame->delay ,"dataGate$o");
        }
        Logger::logFrameSent(simTime().dbl() + networkParams.DD , next_frame_to_send, modifiedMessage, CRC, frame->modificationBit , frame->isLoss, 2, frame->delay);
    }

    // Add Timeout 
    CustomMessage_Base* timeoutMsg = msgToSend->dup();
    // Add the unmodified payload to the timeout message
    timeoutMsg->setPayload(stuffedMessage.c_str());
    startTimer(timeoutMsg, next_frame_to_send  % networkParams.WS);

    // Process Next Message 
    incrementCircular(next_frame_to_send);
    processMessage(next_frame_to_send);

}

void Node::sendTimeoutDataMessage(CustomMessage_Base *msg){
    // isProcessing = false;
    interruptTimeoutNack = false;
    // Extract relevant data from the message
    std::string stuffedMessage = msg->getPayload();
    std::string CRC = Utils::binaryStringFromChar(msg->getTrailer());
    
    msg->setPayload(stuffedMessage.c_str());
    msg->setFrameType(static_cast<int>(FrameType::Data));
    
    // Send the data message
    sendDelayed(msg, networkParams.TD ,"dataGate$o");
    int seqNo = static_cast<int>(msg->getHeader());
    Logger::logFrameSent(simTime().dbl(), seqNo, stuffedMessage, CRC, -1 , false , 0, 0);

}

void Node::processMessage(int index) {
    EV << "isProcessing: " << isProcessing << ", nbuffered: " << nbuffered 
    << ", Index: " << index << ", packets: " << packets.size() << "\n";
    if(isProcessing){
        return;
    }
    // It means I can't process more because the buffer is full or there is no more packets
    if(nbuffered >= networkParams.WS || packets.empty()){
        return ; 
    }
    isProcessing = true;
    
    nbuffered++;

    CustomMessage_Base* customMessage = new CustomMessage_Base();
    
    // Extract first message from lines
    std::string nextLine = packets.front();
    packets.pop_front();
    std::pair<std::string,std::string> extractedMessage = Utils::extractMessage(nextLine); 
    std::string errorNumber = extractedMessage.first;  
    std::string message = extractedMessage.second;  
    // Parse Errors 

    // Stuff the message and compute CRC
    std::string stuffedMessage = Framing::stuff(message); 
    std::string CRC = CRCModule->computeCRC(Utils::stringToBinaryStream(stuffedMessage)); 
    
    Frame * frame = parseFlags(errorNumber,stuffedMessage);
    
    char trailerChar = CRC[0];
    customMessage->setTrailer(trailerChar);
    customMessage->setHeader(static_cast<char>(index));
    
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

    CustomMessage_Base * timeoutMsg = msg->dup();
    int seqNo = static_cast<int>(timeoutMsg->getHeader());
    Logger::logTimeout(simTime().dbl(), seqNo);
    //TODO: Add ScheduleAt with PT to send the Unmodified message
    timeoutMsg->setFrameType(static_cast<int>(FrameType::SendDataTimeout));
    scheduleAt(simTime() + networkParams.PT + 0.001, timeoutMsg);
    timeoutNackFinishTime = simTime().dbl() + networkParams.PT + 0.001;
    interruptTimeoutNack = true; 
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
    newFrame->delay = (errorNumber[3] == '1') ? networkParams.ED : 0;           

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

// -------------------------- Receiver --------------------------------------------------- 

void Node::handleIncomingDataMessage(CustomMessage_Base *receivedMsg) 
{
    // Checking if the received sequence number is in the range 
    int seqNo = static_cast<int>(receivedMsg->getHeader());
    if(!Utils::isSeqNoInRecvWindow(frame_expected, seqNo, too_far)){
        return; 
    }
    //TODO: Add Handling Lost ACK 
    double random = uniform(0, 1);
    bool lostACK = random < networkParams.LP ? true : false;
    receivedMsg->setKind(lostACK);
    // Extract message details
    std::string payload = receivedMsg->getPayload(); 
    char trailerChar = receivedMsg->getTrailer(); 
    std::string trailer = std::string(1, static_cast<char>(trailerChar)); 
 
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
    int seqNo = static_cast<int>(receivedMsg->getHeader());
     EV<<"Invalid CRC Seq No : "<<seqNo<<"  Expected Frame: "<<frame_expected<<"\n";
    if(seqNo == frame_expected && !sentNack[seqNo]){
        sentNack[seqNo] = true;
        CustomMessage_Base* nackMessage = new CustomMessage_Base();
        nackMessage->setFrameType(static_cast<int>(FrameType::NACK));
        nackMessage->setAckNackNumber(seqNo);
        // Add Lost or not in the kind of message 
        nackMessage->setKind(receivedMsg->getKind());
        scheduleAt(simTime() + networkParams.PT, nackMessage);
    }
}

void Node::processValidReceivedMessage(CustomMessage_Base* receivedMsg) 
{
    Frame * frame = new Frame();
    frame->message = receivedMsg;
    int seqNo = static_cast<int>(receivedMsg->getHeader());
    in_buffer[seqNo % networkParams.WS] = frame;
    // Rest SentNack boolean when valid message is received 
    sentNack[seqNo % networkParams.WS] = false;
    EV<<"Received Seq No : "<<seqNo<<"  Expected Frame: "<<frame_expected<< " Too Far : "<<too_far<< "\n";
    bool shouldSendNack = true; 
     // Send ACK to sender 
    bool sendAck = false; 
    while (in_buffer[frame_expected % networkParams.WS] != nullptr) {
        EV<<"Frame: "<<frame_expected<<"\n";
        to_network_layer(in_buffer[frame_expected % networkParams.WS]->message);
        in_buffer[frame_expected % networkParams.WS] = nullptr;
        incrementCircular(frame_expected);
        incrementCircular(too_far);
        sendAck = true; 
        shouldSendNack = false;
    }
     CustomMessage_Base* ackMessage = new CustomMessage_Base();
     int ackNo = frame_expected;
     ackMessage->setAckNackNumber(ackNo);
     if(shouldSendNack && !sentNack[ackNo % networkParams.WS]){
        sentNack[ackNo % networkParams.WS] = true; 
        ackMessage->setFrameType(static_cast<int>(FrameType::NACK));
     }else{
        ackMessage->setFrameType(static_cast<int>(FrameType::ACK));
     }
     // Add Lost or not in the kind of message 
     ackMessage->setKind(receivedMsg->getKind());
     scheduleAt(simTime() + networkParams.PT, ackMessage);
}


void Node::sendAckMessage(CustomMessage_Base *msg){    
    msg->setFrameType(static_cast<int>(FrameType::ACK));
    // Send the data message
    if(!msg->getKind()){ // Check if it's not loss 
        sendDelayed(msg, networkParams.TD ,"dataGate$o");
    }
    Logger::logACK(simTime().dbl(), msg->getAckNackNumber(), true, msg->getKind());
}

void Node::sendNackMessage(CustomMessage_Base *msg){    
    msg->setFrameType(static_cast<int>(FrameType::NACK));
    // Send the data message
    if(!msg->getKind()){ // Check if it's not loss 
        sendDelayed(msg, networkParams.TD ,"dataGate$o");
    }
    Logger::logACK(simTime().dbl(), msg->getAckNackNumber(), false, msg->getKind());
}

void Node::to_network_layer(CustomMessage_Base *receivedMsg){
    int seqNo = static_cast<int>(receivedMsg->getHeader());
    std::string unstuffedMessage = Framing::unstuff(receivedMsg->getPayload());
    Logger::logUpload(simTime().dbl(), unstuffedMessage, seqNo);
}