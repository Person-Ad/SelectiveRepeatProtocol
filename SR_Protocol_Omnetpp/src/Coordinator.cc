//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 
#include <sstream>

#include "Coordinator.h"
#include "Utils.h"
#include "CustomMessage_m.h"

Define_Module(Coordinator);

void Coordinator::initialize()
{
    std::vector<std::string> lines = Utils::readFileLines("../text_files/coordinator.txt");
    std::istringstream stream(lines[0]);

    int node , startTime; 
    stream >> node >> startTime ; 

    // Sending the start time to the correct node
    CustomMessage_Base *customMessage = new CustomMessage_Base();
    customMessage->setPayload(std::to_string(startTime).c_str());
    customMessage->setName(customMessage->getPayload());
    customMessage->setFrameType(3); // Control Type 
    send(customMessage, "controlGate" , node);

}

void Coordinator::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
}
