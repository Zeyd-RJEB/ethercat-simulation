//
// Generated file, do not edit! Created by opp_msgc 4.2 from applicationlayer/EtherCatFrame.msg.
//

#ifndef _ETHERCATFRAME_M_H_
#define _ETHERCATFRAME_M_H_

#include <omnetpp.h>

// opp_msgc version check
#define MSGC_VERSION 0x0402
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of opp_msgc: 'make clean' should help.
#endif



/**
 * Struct generated from applicationlayer/EtherCatFrame.msg by opp_msgc.
 */
struct type12PDU
{
    type12PDU();
    unsigned char CMD;
    unsigned char IDX;
    short ADP;
    short ADO;
    int LEN;
    char reserved;
    bool C;
    bool NEXT;
    short IRQ;
    char DATA;
    short WKC;
};

void doPacking(cCommBuffer *b, type12PDU& a);
void doUnpacking(cCommBuffer *b, type12PDU& a);

/**
 * Class generated from <tt>applicationlayer/EtherCatFrame.msg</tt> by opp_msgc.
 * <pre>
 * packet EtherCatFrame {
 *     int lenght;
 *     char reserved;
 *     char type;
 *     type12PDU pdu[10];
 *     int data;
 *    
 * }
 * </pre>
 */
class EtherCatFrame : public ::cPacket
{
  protected:
    int lenght_var;
    char reserved_var;
    char type_var;
    type12PDU pdu_var[10];
    int data_var;

  private:
    void copy(const EtherCatFrame& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const EtherCatFrame&);

  public:
    EtherCatFrame(const char *name=NULL, int kind=0);
    EtherCatFrame(const EtherCatFrame& other);
    virtual ~EtherCatFrame();
    EtherCatFrame& operator=(const EtherCatFrame& other);
    virtual EtherCatFrame *dup() const {return new EtherCatFrame(*this);}
    virtual void parsimPack(cCommBuffer *b);
    virtual void parsimUnpack(cCommBuffer *b);

    // field getter/setter methods
    virtual int getLenght() const;
    virtual void setLenght(int lenght);
    virtual char getReserved() const;
    virtual void setReserved(char reserved);
    virtual char getType() const;
    virtual void setType(char type);
    virtual unsigned int getPduArraySize() const;
    virtual type12PDU& getPdu(unsigned int k);
    virtual const type12PDU& getPdu(unsigned int k) const {return const_cast<EtherCatFrame*>(this)->getPdu(k);}
    virtual void setPdu(unsigned int k, const type12PDU& pdu);
    virtual int getData() const;
    virtual void setData(int data);
};

inline void doPacking(cCommBuffer *b, EtherCatFrame& obj) {obj.parsimPack(b);}
inline void doUnpacking(cCommBuffer *b, EtherCatFrame& obj) {obj.parsimUnpack(b);}


#endif // _ETHERCATFRAME_M_H_
