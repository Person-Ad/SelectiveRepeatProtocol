/*
 * Utils.h
 *
 *  Created on: Dec 3, 2024
 *      Author: Ahmed_Osamahelmy
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <string>
#include <vector>

class Utils {
public:
    Utils();
        
    static std::vector<std::string> readFileLines(const std::string& filePath);
    static std::pair<std::string, std::string> extractMessage(const std::string& input);
    static std::string stringToBinaryStream(const std::string& input);
    static std::string toBinary4Bits(int number);
    static std::string binaryStringFromChar(char trailer);
    static bool isSeqNoInRecvWindow(int frame_expected, int seqNo, int too_far);
    static bool between(int a, int b, int c) ;
    virtual ~Utils();
};

#endif /* UTILS_H_ */
