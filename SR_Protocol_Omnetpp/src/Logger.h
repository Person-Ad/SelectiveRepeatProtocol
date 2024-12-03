/*
 * Logger.h
 *
 *  Created on: Dec 3, 2024
 *      Author: Ahmed_Osamahelmy
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>


class Logger {
public:
    Logger(const std::string& logFilePath,int senderIndex);
    ~Logger();
    void logTime(double time, const std::string& message);
    void logFrameSent(double time, int seqNum, const std::string& payload, 
                             const std::string& trailer, int modified, bool lost, int duplicate, int delay);
    void logChannelError(double time, const std::string& errorCode);
    void logUpload(double time, const std::string& payload, int seqNum);
    void logACK(double time, int ackNum, bool loss);

    private:
    int getReceiver(int nodeId);
    int senderIndex; 
    int receiverIndex; 

    std::ofstream logFile;
};

#endif /* LOGGER_H_ */
