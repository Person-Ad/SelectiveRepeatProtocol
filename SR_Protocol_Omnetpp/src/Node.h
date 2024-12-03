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

#ifndef __SR_PROTOCOL_OMNETPP_NODE_H_
#define __SR_PROTOCOL_OMNETPP_NODE_H_

#include <omnetpp.h>

using namespace omnetpp;

/**
 * TODO - Generated class
 */
class Node : public cSimpleModule
{
  public:
    int WS = 0;
    double TO = 0;
    double PT = 0;
    double TD = 0;
    double ED = 0;
    double DD = 0;
    double LP = 0;

    bool isSenderNode = false; 
    std::vector<std::string> lines; 
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    void sendDataMessage(std::string messageValue);

};

#endif
