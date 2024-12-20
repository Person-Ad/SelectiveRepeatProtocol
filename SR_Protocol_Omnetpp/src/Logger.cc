/*
 * Logger.cc
 *
 *  Created on: Dec 3, 2024
 *      Author: Ahmed_Osamahelmy
 */


#include "Logger.h"

// Static members initialization
std::ofstream Logger::logFile;
int Logger::senderIndex;
int Logger::receiverIndex;
std::mutex Logger::logFileMutex;

void Logger::initialize(const std::string& logFilePath, int senderIndex) {
    // Open the log file in truncation mode to clear its contents
    logFile.open(logFilePath, std::ios::out | std::ios::trunc);  // Clears the content of the file

    if (!logFile.is_open()) {
        std::cerr << "Error opening log file: " << logFilePath << std::endl;
    }

    Logger::senderIndex = senderIndex;
    Logger::receiverIndex = getReceiver(senderIndex);
}

void Logger::logTime(double time, const std::string& message) {
    std::stringstream logMessage;
    logMessage << "At time [" << time << "], Node[" << senderIndex << "] , " << message;
    log(logMessage.str());
}

void Logger::logFrameSent(double time, int seqNum, const std::string& payload, 
                          const std::string& trailer, int modified, bool lost, int duplicate, int delay) {
    std::stringstream logMessage;
    logMessage << "At time [" << time << "], Node[" << senderIndex << "] [sent] frame with seq_num=[" << seqNum 
               << "] and payload=[" << payload << "] and trailer=[" << trailer << "] , Modified [" 
               << modified << "] , Lost [" << (lost ? "Yes" : "No") << "], Duplicate [" << duplicate 
               << "], Delay [" << delay << "].";
    log(logMessage.str());
}

void Logger::logChannelError(double time, const std::string& errorCode) {
    std::stringstream logMessage;
    logMessage << "At time [" << time << "], Node[" << senderIndex << "] , Introducing channel error with code =[" 
               << errorCode << "] .";
    log(logMessage.str());
}

void Logger::logUpload(double time, const std::string& payload, int seqNum) {
    std::stringstream logMessage;
    logMessage << "Uploading payload = [" << payload << "], Node[" << receiverIndex << "] and seq_num = [" 
               << seqNum << "] to the network layer";
    log(logMessage.str());
}

void Logger::logACK(double time, int ackNum, bool loss) {
    std::stringstream logMessage;
    logMessage << "At time [" << time << "], Node[" << senderIndex << "] Sending [ACK] with number [" 
               << ackNum << "] ,loss [" << (loss ? "Yes" : "No") << "].";
    log(logMessage.str());
}

void Logger::log(const std::string& message) {
    // Lock the mutex to ensure thread-safe access to logFile
    std::lock_guard<std::mutex> lock(logFileMutex);  
    logFile << message << std::endl;
}

void Logger::cleanup() {
    // Close the log file when done
    if (logFile.is_open()) {
        logFile.close();
    }
}

int Logger::getReceiver(int senderIndex) {
    return 1 - senderIndex;  // If senderIndex is 0, receiver is 1, and vice versa
}