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
    static std::pair<int, std::string> extractMessage(const std::string& input);
    virtual ~Utils();
};

#endif /* UTILS_H_ */
