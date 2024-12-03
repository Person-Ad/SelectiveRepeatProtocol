/*
 * Logger.cc
 *
 *  Created on: Dec 3, 2024
 *      Author: Ahmed_Osamahelmy
 */


#include "Logger.h"

std::ofstream Logger::logFile;

Logger::Logger(const std::string& logFilePath) {
    // Open the log file in append mode
    logFile.open(logFilePath, std::ios::app);
    if (!logFile.is_open()) {
        std::cerr << "Error opening log file: " << logFilePath << std::endl;
    }
}

void Logger::logTime(double time, int nodeId, const std::string& message) {
    std::stringstream logMessage;
    logMessage << "At time [" << time << "], Node[" << nodeId << "] , " << message;
    logFile << logMessage.str() << std::endl;
}

void Logger::logFrameSent(double time, int nodeId, int seqNum, const std::string& payload, 
                          const std::string& trailer, int modified, bool lost, int duplicate, int delay) {
    std::stringstream logMessage;
    logMessage << "At time [" << time << "], Node[" << nodeId << "] [sent] frame with seq_num=[" << seqNum 
               << "] and payload=[" << payload << "] and trailer=[" << trailer << "] , Modified [" 
               << modified << "] , Lost [" << (lost ? "Yes" : "No") << "], Duplicate [" << duplicate 
               << "], Delay [" << delay << "].";
    logFile << logMessage.str() << std::endl;
}

void Logger::logChannelError(double time, int nodeId, const std::string& errorCode) {
    std::stringstream logMessage;
    logMessage << "At time [" << time << "], Node[" << nodeId << "] , Introducing channel error with code =[" 
               << errorCode << "] .";
    logFile << logMessage.str() << std::endl;
}

void Logger::logUpload(double time, int nodeId, const std::string& payload, int seqNum) {
    std::stringstream logMessage;
    logMessage << "Uploading payload = [" << payload << "], Node[" << nodeId << "] and seq_num = [" 
               << seqNum << "] to the network layer";
    logFile << logMessage.str() << std::endl;
}

void Logger::logACK(double time, int nodeId, int ackNum, bool loss) {
    std::stringstream logMessage;
    logMessage << "At time [" << time << "], Node[" << nodeId << "] Sending [ACK] with number [" 
               << ackNum << "] ,loss [" << (loss ? "Yes" : "No") << "].";
    logFile << logMessage.str() << std::endl;
}

Logger::~Logger() {
    // Close the log file when the logger object is destroyed
    if (logFile.is_open()) {
        logFile.close();
    }
}

int Logger::getReceiver(int nodeId) {
    return 1 - nodeId;  // If nodeId is 0, receiver is 1, and vice versa
}