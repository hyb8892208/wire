
#ifndef _ETH_SVC_RAWETHSOCKET_H_
#define _ETH_SVC_RAWETHSOCKET_H_

// used to send ethernet frame
class TxEtherSocket
{
private:
    string      _ifName;
    EtherMac    _myMac;
    int         _sockfd;
    bool        _isOpen;
    int         _ifIndex;

public:
    explicit TxEtherSocket(void);
    ~TxEtherSocket()    { Close(); };

    int Open(unsigned char *ifName);
    void  Close(void);

    bool IsOpen(void)   {   return _isOpen; };

    EtherMac GetMac(void)   {   return _myMac;  };

    int SendTo(EtherMac& dstMac, RawEtherFrame& frame);
    int Send(RawEtherFrame& frame);

    int         _debug;
};

// used to receive ethernet frame
class RxEtherSocket
{
private:
    string      _ifName;
    EtherMac    _myMac;
    int         _sockfd;
    int         _isOpen;
    unsigned short _ethType;
    
public:
    explicit RxEtherSocket(void);
    ~RxEtherSocket() { Close(); };

    int Open(unsigned char* ifName, unsigned short eth_p);
    void Close(void);

    bool IsOpen(void)   {   return _isOpen; };

    EtherMac GetMac(void)   {   return _myMac;  };

    int Recv(RawEtherFrame& frame);

    int Sockfd(void) {  return _sockfd; };

    int         _debug;

};

#endif //_ETH_SVC_RAWETHSOCKET_H_
