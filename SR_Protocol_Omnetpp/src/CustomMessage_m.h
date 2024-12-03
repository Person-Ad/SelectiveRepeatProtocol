//
// Generated file, do not edit! Created by nedtool 5.6 from CustomMessage.msg.
//

#ifndef __CUSTOMMESSAGE_M_H
#define __CUSTOMMESSAGE_M_H

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <omnetpp.h>

// nedtool version check
#define MSGC_VERSION 0x0506
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of nedtool: 'make clean' should help.
#endif



/**
 * Class generated from <tt>CustomMessage.msg:19</tt> by nedtool.
 * <pre>
 * //
 * // TODO generated message class
 * //
 * packet CustomMessage
 * {
 *     \@customize(true);  // see the generated C++ header for more info
 * 
 *     char header;        // Data sequence number
 *     string payload;    // Message contents after byte stuffing (as characters)
 *     char trailer;      // Parity byte
 *     int frameType;     // Frame type: Data=2, ACK=1, NACK=0
 *     int ackNackNumber; // ACK/NACK number
 * }
 * </pre>
 *
 * CustomMessage_Base is only useful if it gets subclassed, and CustomMessage is derived from it.
 * The minimum code to be written for CustomMessage is the following:
 *
 * <pre>
 * class CustomMessage : public CustomMessage_Base
 * {
 *   private:
 *     void copy(const CustomMessage& other) { ... }

 *   public:
 *     CustomMessage(const char *name=nullptr, short kind=0) : CustomMessage_Base(name,kind) {}
 *     CustomMessage(const CustomMessage& other) : CustomMessage_Base(other) {copy(other);}
 *     CustomMessage& operator=(const CustomMessage& other) {if (this==&other) return *this; CustomMessage_Base::operator=(other); copy(other); return *this;}
 *     virtual CustomMessage *dup() const override {return new CustomMessage(*this);}
 *     // ADD CODE HERE to redefine and implement pure virtual functions from CustomMessage_Base
 * };
 * </pre>
 *
 * The following should go into a .cc (.cpp) file:
 *
 * <pre>
 * Register_Class(CustomMessage)
 * </pre>
 */
class CustomMessage_Base : public ::omnetpp::cPacket
{
  protected:
    char header;
    ::omnetpp::opp_string payload;
    char trailer;
    int frameType;
    int ackNackNumber;

  private:
    void copy(const CustomMessage_Base& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const CustomMessage_Base&);
    // make constructors protected to avoid instantiation
    // make assignment operator protected to force the user override it
    CustomMessage_Base& operator=(const CustomMessage_Base& other);

  public:
    virtual ~CustomMessage_Base();
    virtual CustomMessage_Base *dup() const override {throw omnetpp::cRuntimeError("You forgot to manually add a dup() function to class CustomMessage");}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    CustomMessage_Base(const char *name=nullptr, short kind=0);
    CustomMessage_Base(const CustomMessage_Base& other);
    // field getter/setter methods
    virtual char getHeader() const;
    virtual void setHeader(char header);
    virtual const char * getPayload() const;
    virtual void setPayload(const char * payload);
    virtual char getTrailer() const;
    virtual void setTrailer(char trailer);
    virtual int getFrameType() const;
    virtual void setFrameType(int frameType);
    virtual int getAckNackNumber() const;
    virtual void setAckNackNumber(int ackNackNumber);
};


#endif // ifndef __CUSTOMMESSAGE_M_H

