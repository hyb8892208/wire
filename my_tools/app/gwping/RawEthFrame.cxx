#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <string>
using namespace std;

#include "config.h"
#include "RawEthFrame.hpp"



//======================================================================================
//
//======================================================================================
EtherMac::EtherMac(void)
{
    memset(_mac, 0, sizeof(_mac));
}

EtherMac::EtherMac(unsigned char* mac)
{
    memcpy(_mac, mac, ETH_ALEN);
}

void EtherMac::SetMac(const unsigned char* mac)
{
    memcpy(_mac, mac, ETH_ALEN);
}

string EtherMac::AsString(void)
{
    string s;

    for(int i=0; i<ETH_ALEN; i++) {
        char buf[3];
        sprintf(buf, "%02x", _mac[i]&0xff);
        if(i!=0)
            s +=  ":";
        s += buf;
    }
    return s;
}

//======================================================================================
//
//======================================================================================
RawEtherFrame::RawEtherFrame(void)
 : _frameSize(ETH_ZLEN) /* default frame size is minimal allowed ethernet frame size */
{
    memset(_frame, 0, ETH_FRAME_LEN );
}

void RawEtherFrame::SetDestMac(EtherMac& mac)
{
    memcpy(_frame, mac.GetMac(), ETH_ALEN);
}

EtherMac RawEtherFrame::GetDestMac(void)
{
    EtherMac mac(_frame);
    return mac;
}

void RawEtherFrame::SetSrcMac(EtherMac& mac)
{
    memcpy(_frame + ETH_ALEN, mac.GetMac(), ETH_ALEN);
}

EtherMac RawEtherFrame::GetSrcMac(void)
{
    EtherMac mac(_frame+ETH_ALEN);
    return mac;
}

void RawEtherFrame::SetType(unsigned short type)
{
    unsigned short* dst_type = (unsigned short*)(_frame + ETH_ALEN + ETH_ALEN);
    *dst_type = htons(type);
}

unsigned short RawEtherFrame::GetType(void)
{
    unsigned short* dst_type = (unsigned short*)(_frame + ETH_ALEN + ETH_ALEN);
    return ntohs(*dst_type);
}

void RawEtherFrame::SetFrameSize(int size)
{
    assert(size>=ETH_HLEN);
    assert(size<=ETH_FRAME_LEN);

    _frameSize = size;
}

void RawEtherFrame::SetDataSize(int size)
{
    int frame_size = size + ETH_HLEN;

    SetFrameSize(frame_size);
}

//======================================================================================
//
//======================================================================================
int EthBufCoder::EncodeDWord(unsigned int data)
{
    if( (_index+sizeof(data)) <= _rawEthFrame.GetMaxDataSize() ) {
        *((unsigned int*)(_rawEthFrame.DataBuf() + _index)) = htonl(data);
        _index += sizeof(data);
        return sizeof(data);
    } else
        return 0;
}

int EthBufCoder::EncodeWord(unsigned short data)
{
    if( (_index+sizeof(data)) <= _rawEthFrame.GetMaxDataSize() ) {
        *((unsigned short*)(_rawEthFrame.DataBuf() + _index)) = htons(data);
        _index += sizeof(data);
        return sizeof(data);
    } else
        return 0;
}

int EthBufCoder::EncodeByte(unsigned char data)
{
    if( (_index+sizeof(data)) <= _rawEthFrame.GetMaxDataSize() ) {
        *(_rawEthFrame.DataBuf() + _index) = data;
        _index += sizeof(data);
        return sizeof(data);
    } else
        return 0;
}

int EthBufCoder::EncodeBuf(unsigned char* buf, int size)
{
    if( (_index + size) <= _rawEthFrame.GetMaxDataSize() ) {
        memcpy(_rawEthFrame.DataBuf() + _index, buf, size);
        _index += size;
        return size;
    } else
        return 0;
}

int EthBufCoder::EncodeCStr(unsigned char* buf)
{
    assert(buf != NULL);

    int real_size = strlen((const char* )buf) + 1;
    return EncodeBuf(buf, real_size);
}

int EthBufCoder::SetWord(int pos, unsigned short data)
{
    if( (pos+sizeof(data)) <= _rawEthFrame.GetMaxDataSize() ) {
        *((unsigned short*)(_rawEthFrame.DataBuf() + pos)) = htons(data);
        return sizeof(data);
    } else
        return 0;
}

//======================================================================================
//
//======================================================================================
int EthBufDecoder::DecodeDWord(unsigned int& value)
{
    if( _index <= (_rawEthFrame.GetDataSize() - sizeof(value)) ){
        value = ntohl( *((unsigned int*)(_rawEthFrame.DataBuf() + _index)));
        _index += sizeof(value);
        return sizeof(value);
    } else
        return 0;
}

int EthBufDecoder::DecodeWord(unsigned short& value)
{
    if( _index <= (_rawEthFrame.GetDataSize() - sizeof(value)) ){
        value = ntohs( *((unsigned short*)(_rawEthFrame.DataBuf() + _index)));
        _index += sizeof(value);
        return sizeof(value);
    } else
        return 0;
}

int EthBufDecoder::DecodeByte(unsigned char& value)
{
    if( _index <= (_rawEthFrame.GetDataSize() - sizeof(value)) ){
        value = *(_rawEthFrame.DataBuf() + _index);
        _index += sizeof(value);
        return sizeof(value);
    } else
        return 0;
}

int EthBufDecoder::DecodeCStr(unsigned char* buf, int size)
{
    assert(buf!=NULL);
    assert(size>1);

    int tail_zero = 0;
    int myindex = _index;

    if( _index <= _rawEthFrame.GetDataSize()  ){
        unsigned char* src = _rawEthFrame.DataBuf();
        int i, j;
        
        for(j=0; myindex<_rawEthFrame.GetDataSize(); myindex++, j++) {
            buf[j] = src[myindex];
            if(src[myindex] == 0) {
                tail_zero = 1;  /* found tail_zero */
                myindex++;
                break;
            }
            if( j == (size-1))
                break;
        }

        if(!tail_zero)  // buf size not enough or no cstr in buffer
            return 0;
        else {
            _index = myindex;
            return j-1; // strlen of the cstr.
        }
    } else
        return 0;
}

int EthBufDecoder::DecodeBuf(unsigned char* buf, int size)
{
    if( _index <= (_rawEthFrame.GetDataSize() - size ) ){
        memcpy(buf, _rawEthFrame.DataBuf() + _index, size);
        _index += size;
        return size;
    } else
        return 0;
}
