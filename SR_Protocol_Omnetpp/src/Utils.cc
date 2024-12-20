/*
 * Utils.cc
 *
 *  Created on: Dec 3, 2024
 *      Author: Ahmed_Osamahelmy
 */
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <bitset>

#include "Utils.h"

Utils::Utils() {
    // TODO Auto-generated constructor stub

}


std::vector<std::string> Utils::readFileLines(const std::string& filePath) {
    std::vector<std::string> lines;
    std::ifstream file(filePath);

    if (!file.is_open()) {
        throw std::runtime_error("Error: Unable to open file " + filePath);
    }

    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }

    file.close();
    return lines;
}

std::pair<int, std::string> Utils::extractMessage(const std::string& input){
    std::istringstream stream(input);
    int errorNumber;
    std::string sentence;

    // Read the errorNumber first
    stream >> errorNumber;

    // Get the rest of the string (after the first space)
    std::getline(stream, sentence);
    
    // Trim leading spaces from the sentence (optional, if needed)
    if (!sentence.empty() && sentence[0] == ' ') {
        sentence = sentence.substr(1);
    }

    return {errorNumber, sentence};
}

std::string Utils::stringToBinaryStream(const std::string& input) {
    std::string binaryStream;

    // Convert each character in the input string to its binary representation
    for (char c : input) {
        std::bitset<8> bits(c); // Convert each character to 8-bit binary
        binaryStream += bits.to_string(); // Append binary representation to the stream
    }

    return binaryStream;
}
std::string Utils::toBinary4Bits(int number) {
    // Ensure the number fits in 4 bits and convert to binary using std::bitset
    return std::bitset<4>(number).to_string();
}
std::string Utils::binaryStringFromChar(char trailer) {
    std::bitset<8> bits(static_cast<unsigned char>(trailer));
    return bits.to_string();
}
bool Utils::between(int a, int b, int c) {
    return ((a <= b && b < c) || (c < a && a <= b) || (b < c && c < a));
}
Utils::~Utils() {
    // TODO Auto-generated destructor stub
}

