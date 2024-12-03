/*
 * ErrorDetection.h
 *
 *  Created on: Dec 3, 2024
 *      Author: Ahmed_Osamahelmy
 */

#ifndef ERRORDETECTION_H_
#define ERRORDETECTION_H_

#include <iostream>
#include <string>

class ErrorDetection {
private: 
    std::string polynomialGenerator;
public:
    ErrorDetection(const std::string & generator);
    std::string computeCRC(const std::string &data);
    bool validateCRC(const std::string &dataWithCRC);
    virtual ~ErrorDetection();
};

#endif /* ERRORDETECTION_H_ */
