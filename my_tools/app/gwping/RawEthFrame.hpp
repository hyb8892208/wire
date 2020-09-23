
#ifndef _ETH_SVC_RAWETHFRAME_H_
#define _ETH_SVC_RAWETHFRAME_H_

// handle ethernet mac address
class EtherMac
{
private:
    unsigned char _mac[ETH_ALEN];
public:
    explicit EtherMac(void);       // initial a NULL mac;
    explicit EtherMac(unsigned char* mac);  // mac from a char array

    unsigned char* GetMac(void)  {   return _mac;    };
    void SetMac(const unsigned char* mac);
    bool isBroadCast(void) 
    {
        int cnt = 0;
        for(int i=0; i<ETH_ALEN; i++)
            if(_mac[i]!=0xff)   cnt++;

        if(cnt)
            return false;
        else
            return true;
    };

    string AsString(void);

    bool isEqual(EtherMac& m) 
    {
        if( memcmp(_mac, m._mac, ETH_ALEN)==0)
            return true;
        else
            return false;
    };
};

// An RawEtherFrame include destmac, srcmac, ethernet type, and 42 to 1500 bytes payload
class RawEtherFrame
{
private:
    unsigned char _frame[ETH_FRAME_LEN];     // the whole payload;
    int _frameSize;
    
public:
    explicit RawEtherFrame(void);

    void SetDestMac(EtherMac& mac);
    EtherMac GetDestMac(void);

    void SetSrcMac(EtherMac& mac);
    EtherMac GetSrcMac(void);
    
    void SetType(unsigned short type);
    unsigned short GetType(void);

    int GetMaxFrameSize(void)       { return ETH_FRAME_LEN;  };             /* max frame buffer size */
    int GetFrameSize(void)          { return _frameSize; };                 /* get actual frame size */
    int GetMaxDataSize(void)        { return ETH_FRAME_LEN - ETH_HLEN; };   /* max allowed data size */
    int GetDataSize(void)           { return _frameSize - ETH_HLEN; };      /* actuall data size in this frame */
    
    unsigned char* RawBuf(void)     { return _frame;  };
    unsigned char* DataBuf(void)    { return _frame + ETH_HLEN; };
    void SetFrameSize(int size);
    void SetDataSize(int size);
};

class EthBufCoder
{
private:
    RawEtherFrame& _rawEthFrame;
    int   _index;

public:
    explicit EthBufCoder(RawEtherFrame& f)
        : _rawEthFrame(f), _index(0)
    {};
    
    int GetMaxSize(void)   {   return _rawEthFrame.GetMaxDataSize();    };
    int GetDataSize(void)  {   return _index;       };

    int GetFreeSize(void)   { return GetMaxSize() - GetDataSize();      };
    void Reset(void)        { _index = 0;   };

    int EncodeDWord(unsigned int data);
    int EncodeWord(unsigned short data);
    int EncodeByte(unsigned char data);
    int EncodeBuf(unsigned char* buf, int size);

    int EncodeCStr(unsigned char* buf);

    int SetWord(int pos, unsigned short data);
};

class EthBufDecoder
{
private:
    RawEtherFrame& _rawEthFrame;
    int   _index;

public:
    explicit EthBufDecoder(RawEtherFrame& f)
        : _rawEthFrame(f), _index(0)
    {};

    int GetDataSize(void)  {   return _rawEthFrame.GetDataSize();       };

    int GetLeftSize(void)   { return GetDataSize() - _index;      };
    void Reset(void)        { _index = 0;   };

    int DecodeDWord(unsigned int& value);
    int DecodeWord(unsigned short& value);
    int DecodeByte(unsigned char& value);
    int DecodeCStr(unsigned char* buf, int size);
    int DecodeBuf(unsigned char* buf, int size);
};

#endif //_ETH_SVC_RAWETHFRAME_H_
