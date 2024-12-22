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
static const unsigned char INIT_CRC = 0xFF;    // Initial CRC value
static const unsigned char MSB_MASK = 0x80;    // Most Significant Bit mask (10000000)
static const unsigned char GEN_POLY = 0x07;    // Generator polynomial (like 0x07 for CRC-8)
static const int BITS_PER_BYTE = 8;

std::string ErrorDetection::computeCRC(const std::string &data) {
    unsigned char crc = INIT_CRC;
    
    for (unsigned char c : data) {
        crc ^= c;
        // Process each bit
        for (int i = 0; i < BITS_PER_BYTE; ++i) {
            bool hasMSB = (crc & MSB_MASK);
            crc = hasMSB ? ((crc << 1) ^ GEN_POLY) : (crc << 1);
        }
    }
    
    return std::string(1, static_cast<char>(crc));
}

bool ErrorDetection::validateCRC(const std::string &dataWithCRC) {
   // Extract data and received CRC 
   std::string data = dataWithCRC.substr(0, dataWithCRC.length() - 1);
   unsigned char receivedCRC = static_cast<unsigned char>(dataWithCRC.back());
   
   // Calculate expected CRC using computeCRC
   std::string calculatedCRC = computeCRC(data);
   
   // Compare calculated CRC with received CRC
   return (static_cast<unsigned char>(calculatedCRC[0]) == receivedCRC);
}

ErrorDetection::~ErrorDetection() {
    // TODO Auto-generated destructor stub
}

