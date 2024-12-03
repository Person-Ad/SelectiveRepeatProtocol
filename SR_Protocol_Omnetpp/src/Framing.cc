/*
 * Framing.cc
 *
 *  Created on: Dec 3, 2024
 *      Author: Ahmed_Osamahelmy
 */

#include "Framing.h"


Framing::Framing() {
    // TODO Auto-generated constructor stub

}

std::string Framing::stuff(const std::string& payload) {
    std::string stuffedMessage ; 
    stuffedMessage += flagByte;  

    for (char ch : payload) {
        if (ch == flagByte || ch == escapeByte) {
            stuffedMessage += escapeByte;  
        }
        stuffedMessage += ch;  
    }

    stuffedMessage += flagByte; 
    return stuffedMessage;
}

std::string Framing::unstuff(const std::string& stuffedPayload) {
    std::string unstuffedMessage;

    // Ensure the message starts and ends with the flag byte flagByte
    if (stuffedPayload.front() != flagByte || stuffedPayload.back() != flagByte) {
        std::cerr << "Invalid stuffed message: Missing flag bytes." << std::endl;
        return "";
    }

    std::string payload = stuffedPayload.substr(1, stuffedPayload.length() - 2);

    for (size_t i = 0; i < payload.length(); ++i) {
        if (payload[i] == escapeByte) {
            if (i + 1 < payload.length()) {
                unstuffedMessage += payload[i + 1];  
                ++i;  
            }
        } else {
            unstuffedMessage += payload[i];  // Add the character as is
        }
    }

    return unstuffedMessage;
}

Framing::~Framing() {
    // TODO Auto-generated destructor stub
}

