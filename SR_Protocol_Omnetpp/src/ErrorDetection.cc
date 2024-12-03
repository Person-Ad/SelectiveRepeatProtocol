/*
 * ErrorDetection.cc
 *
 *  Created on: Dec 3, 2024
 *      Author: Ahmed_Osamahelmy
 */

#include "ErrorDetection.h"

ErrorDetection::ErrorDetection(const std::string & generator) {
    // TODO Auto-generated constructor stub
    polynomialGenerator = generator;
}
std::string ErrorDetection::computeCRC(const std::string &data) {
    std::string augmentedData = data;
    augmentedData.append(polynomialGenerator.length() - 1, '0'); 

    std::string remainder = augmentedData;

    for (size_t i = 0; i <= augmentedData.length() - polynomialGenerator.length(); ++i) {
        if (remainder[i] == '1') {
            for (size_t j = 0; j < polynomialGenerator.length(); ++j) {
                remainder[i + j] = (remainder[i + j] == polynomialGenerator[j]) ? '0' : '1';
            }
        }
    }

    return remainder.substr(remainder.length() - (polynomialGenerator.length() - 1));
}

bool ErrorDetection::validateCRC(const std::string &dataWithCRC) {
    std::string remainder = dataWithCRC;

    for (size_t i = 0; i <= dataWithCRC.length() - polynomialGenerator.length(); ++i) {
        if (remainder[i] == '1') {
            for (size_t j = 0; j < polynomialGenerator.length(); ++j) {
                remainder[i + j] = (remainder[i + j] == polynomialGenerator[j]) ? '0' : '1';
            }
        }
    }

    // If remainder is all zeros, CRC is valid
    return remainder.substr(remainder.length() - (polynomialGenerator.length() - 1)) == std::string(polynomialGenerator.length() - 1, '0');
}

ErrorDetection::~ErrorDetection() {
    // TODO Auto-generated destructor stub
}

