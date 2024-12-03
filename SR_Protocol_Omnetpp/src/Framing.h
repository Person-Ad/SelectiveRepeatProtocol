/*
 * Framing.h
 *
 *  Created on: Dec 3, 2024
 *      Author: Ahmed_Osamahelmy
 */

#ifndef FRAMING_H_
#define FRAMING_H_
#include <iostream>
#include <string>

class Framing {
private: 
    static const char flagByte = '$'; 
    static const char escapeByte = '/'; 
    
public:
    Framing();
    static std::string stuff(const std::string& payload);
    static std::string unstuff(const std::string& stuffedPayload);
    virtual ~Framing();
};

#endif /* FRAMING_H_ */
