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
#include <mutex>

class Logger {
public:
    static void initialize(const std::string& logFilePath, int senderIndex);
    static void logTime(double time, const std::string& message);
    static void logFrameSent(double time, int seqNum, const std::string& payload, 
                             const std::string& trailer, int modified, bool lost, int duplicate, int delay);
    static void logChannelError(double time, const std::string& errorCode);
    static void logUpload(double time, const std::string& payload, int seqNum);
    static void logACK(double time, int ackNum, bool loss);
    static void log(const std::string& message);
    static void cleanup();  // Method to clean up the resources when done

    // Destructor will no longer be needed for static methods

private:
    static std::ofstream logFile;
    static int senderIndex;
    static int receiverIndex;
    static std::mutex logFileMutex;

    static int getReceiver(int senderIndex);
};

#endif /* LOGGER_H_ */
