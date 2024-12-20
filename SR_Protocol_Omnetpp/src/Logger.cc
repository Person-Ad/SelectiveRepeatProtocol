/*
 * Logger.cc
 *
 *  Created on: Dec 3, 2024
 *      Author: Ahmed_Osamahelmy
 */


#include "Logger.h"

Logger::Logger(const std::string& logFilePath, int senderIndex) {
    // Open the log file in truncation mode to clear its contents
    logFile.open(logFilePath, std::ios::out | std::ios::trunc);  // Clears the content of the file

    if (!logFile.is_open()) {
        std::cerr << "Error opening log file: " << logFilePath << std::endl;
    }

    this->senderIndex = senderIndex;
    this->receiverIndex = getReceiver(senderIndex);
}

void Logger::logTime(double time, const std::string& message) {
    std::stringstream logMessage;
    logMessage << "At time [" << time << "], Node[" << senderIndex << "] , " << message;
    logFile << logMessage.str() << std::endl;
}

void Logger::logFrameSent(double time, int seqNum, const std::string& payload, 
                          const std::string& trailer, int modified, bool lost, int duplicate, int delay) {
    std::stringstream logMessage;
    logMessage << "At time [" << time << "], Node[" << senderIndex << "] [sent] frame with seq_num=[" << seqNum 
               << "] and payload=[" << payload << "] and trailer=[" << trailer << "] , Modified [" 
               << modified << "] , Lost [" << (lost ? "Yes" : "No") << "], Duplicate [" << duplicate 
               << "], Delay [" << delay << "].";
    logFile << logMessage.str() << std::endl;
}

void Logger::logChannelError(double time, const std::string& errorCode) {
    std::stringstream logMessage;
    logMessage << "At time [" << time << "], Node[" << senderIndex << "] , Introducing channel error with code =[" 
               << errorCode << "] .";
    logFile << logMessage.str() << std::endl;
}

void Logger::logUpload(double time, const std::string& payload, int seqNum) {
    std::stringstream logMessage;
    logMessage << "Uploading payload = [" << payload << "], Node[" << senderIndex << "] and seq_num = [" 
               << seqNum << "] to the network layer";
    logFile << logMessage.str() << std::endl;
}

void Logger::logACK(double time, int ackNum, bool loss) {
    std::stringstream logMessage;
    logMessage << "At time [" << time << "], Node[" << senderIndex << "] Sending [ACK] with number [" 
               << ackNum << "] ,loss [" << (loss ? "Yes" : "No") << "].";
    logFile << logMessage.str() << std::endl;
}

Logger::~Logger() {
    // Close the log file when the logger object is destroyed
    if (logFile.is_open()) {
        logFile.close();
    }
}

int Logger::getReceiver(int senderIndex) {
    return 1 - senderIndex;  // If senderIndex is 0, receiver is 1, and vice versa
}