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
    Logger(const std::string& logFilePath);
    ~Logger();
    static void logTime(double time, int nodeId, const std::string& message);
    static void logFrameSent(double time, int nodeId, int seqNum, const std::string& payload, 
                             const std::string& trailer, int modified, bool lost, int duplicate, int delay);
    static void logChannelError(double time, int nodeId, const std::string& errorCode);
    static void logUpload(double time, int nodeId, const std::string& payload, int seqNum);
    static void logACK(double time, int nodeId, int ackNum, bool loss);

    private:
    static int getReceiver(int nodeId);
    
    static std::ofstream logFile;
};

#endif /* LOGGER_H_ */
