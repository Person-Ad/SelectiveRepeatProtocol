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


Utils::~Utils() {
    // TODO Auto-generated destructor stub
}

