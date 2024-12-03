/*
 * Utils.cpp
 *
 *  Created on: Dec 3, 2024
 *      Author: Ahmed_Osamahelmy
 */
#include <fstream>
#include <iostream>
#include <stdexcept>

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


Utils::~Utils() {
    // TODO Auto-generated destructor stub
}

