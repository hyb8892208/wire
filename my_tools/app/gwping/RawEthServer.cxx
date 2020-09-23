
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <netinet/ether.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

extern "C" {
    #include "iniparser.h"
}; // extern "C"


#include <string>
#include <vector>
using namespace std;

#include "config.h"
#include "RawEthFrame.hpp"
#include "RawEthSocket.hpp"


typedef vector<string> TStringVector;

class ServerInfoItem
{
public:
    bool _isCmd;
    string _key;
    string _value;
};


typedef vector<ServerInfoItem>  TServerInfoVector;

class ClientConnection
{
public:
    time_t          _lastActive;    // last active time of client
    EtherMac        _mac;           // mac addreess of the client
    char            _unique_id[16]; // unique_id of last packet
};
typedef vector<ClientConnection> TClientConnVector;

class RawEthServer
{
public:
    string              _ifName;
    TStringVector       _cmdList;
    TServerInfoVector   _serverInfoList;
    TxEtherSocket       _txSocket;
    RxEtherSocket       _rxSocket;
    TClientConnVector   _clients;
    string              _blackList;

    // debug flags;
    int _debug;
    int _verbose;
    int _dump;
    int _dumpbin;

    explicit RawEthServer(void)
        :_debug(0), _verbose(0), _dump(0), _dumpbin(0)
    {};

    int processIniFile(char* iniName);
    int init(char* ifName, char* iniName);
    void Close(void);

    int Recv(RawEtherFrame& frame)
    {
        int ret;
        if(_rxSocket.IsOpen()) {
            ret = _rxSocket.Recv(frame);
            if( (ret > 0) && _dumpbin )
                dump_packet_binary(frame);
            return ret;
        }
        else
            return -1;
    };

    int ProcessPacket(RawEtherFrame& frame);

private:
    void dump_packet_binary(RawEtherFrame& frame)
    {
        int numbytes = frame.GetFrameSize();
        esvc_log("Dump packet, size is %d\n", numbytes);
        for(int i=0; i<numbytes; i++) {
            if( (i%8==7) )
                esvc_log("\n");
            else
                esvc_log("0x%02x ", frame.RawBuf()[i] & 0xff);
        }
        esvc_log("\n");
    };

    int isValidPacket(RawEtherFrame& frame);    // return 0 if it is a valid packet, else < 0
    void dump_packet_svc(RawEtherFrame& frame);
    TClientConnVector::iterator UpdateClient(RawEtherFrame& frame);

    int exec_cmd(string& cmd, string& result);
    int inBlackList(unsigned char* buf);
};


int RawEthServer::processIniFile(char* iniName)
{
    dictionary* ini ;        
    int i, length;
    char *p, *p1;

    ini = iniparser_load(iniName);
    /* Load the ini file from iniName or return. */
    if ( ini==NULL ) {
        esvc_log(ESVC_ERR "open config file [%s] failed\n", iniName);
        return -1;
    }
    
    /* load all allowed command */
    p = iniparser_getstring(ini, "general:allow", NULL);
    if( NULL==p ) {
        esvc_log(ESVC_ERR "config file do not have allow item\n");
        return -2;
    }
    length = strlen(p);
    if(length<=0){
        esvc_log(ESVC_ERR "config file allow section is empty, discarded\n");
        return -3;
    }
    
    p1 = strdup(p);
    p = strtok(p1, ",");
    while(1) {
        if(p)
            _cmdList.push_back(p);
        else
            break;
        p = strtok(NULL, ",");
    }
    free(p1);
    esvc_log(ESVC_INFO "support %d commands:", _cmdList.size());
    for(TStringVector::iterator iter = _cmdList.begin(); iter!=_cmdList.end(); iter++) {
        esvc_log("%s,", iter->c_str());
    }
    esvc_log("\n");
        

    int num_of_serverinfo_cmds = iniparser_getsecnkeys(ini, "serverinfo");
    if ( num_of_serverinfo_cmds <= 0 ) {
        esvc_log(ESVC_ERR "error load serverinfo section\n");
        _cmdList.clear();
        return -4;
    }

    esvc_log(ESVC_INFO "Server info:\n");
    char** pInfo = iniparser_getseckeys(ini, "serverinfo");
    for(i=0; i<num_of_serverinfo_cmds; i++) {
        ServerInfoItem sii;
        sii._key = pInfo[i];
        sii._value =  iniparser_getstring(ini, pInfo[i], "");
        if(sii._value.length() > 0) {
            if(sii._value[0] == '$') {
                sii._isCmd = false;
                sii._value.erase(0, 1);
            } else
                sii._isCmd = true;
             
            _serverInfoList.push_back(sii);
            esvc_log(ESVC_INFO "%s [%s]=[%s]\n", sii._isCmd ? "CMD" : "VAR", sii._key.c_str(), sii._value.c_str());
        } else 
            esvc_log(ESVC_WARN "section[serverinfo] item[%s] is empty\n", sii._key.c_str());
    }

    _debug =  iniparser_getint(ini, "general:debug", 0);
    _verbose =  iniparser_getint(ini, "general:verbose", 0);
    _dump =  iniparser_getint(ini, "general:dump", 0);
    _dumpbin =  iniparser_getint(ini, "general:dumpbin", 0);
    esvc_log(ESVC_INFO "debug is %d, verbose s %d, dump is %d, dumpbin is %d\n", _debug, _verbose, _dump, _dumpbin);
    _blackList = iniparser_getstring(ini, "general:blacklist", NULL);
    if( iniparser_getint(ini, "general:blackslash", 1) )
        _blackList += "\\";
    if( iniparser_getint(ini, "general:blacksemicolon", 1) )
        _blackList += ";";
    if( iniparser_getint(ini, "general:blackpound", 1) )
        _blackList += "#";
    esvc_log(ESVC_INFO "blacklist is %s\n", _blackList.c_str());

    iniparser_freedict(ini);

    return 0;
}

int RawEthServer::init(char* ifName, char* iniName)
{
    int i;

    i = processIniFile(iniName);
    if(i<0)
        return i;

    i = _txSocket.Open((unsigned char*)ifName);
    if(i<0) {
        _cmdList.clear();
        _serverInfoList.clear();
        return i - 10;
    }

    i = _rxSocket.Open((unsigned char*)ifName, _ETHER_TYPE_OPVX);
    if(i<0) {
        _cmdList.clear();
        _serverInfoList.clear();
        _txSocket.Close();
        return i - 20;
    }
    if(_debug) {
        esvc_log(ESVC_INFO " Mac %s\n", _txSocket.GetMac().AsString().c_str());
        _rxSocket._debug = 1;
        _txSocket._debug = 1;
    }

    return 0;
}

void RawEthServer::Close(void)
{
    _ifName = "";
    _cmdList.clear();
    _serverInfoList.clear();
}


int RawEthServer::isValidPacket(RawEtherFrame& frame)
{
    struct RawSvcHead head;
    int size;

    size = frame.GetFrameSize();
    if( size < (ETH_HLEN + sizeof(struct RawSvcHead)) ) {
        esvc_log(ESVC_ERR " packet size(%d) < minimal size(%d)\n", size, ETH_HLEN + sizeof(RawSvcHead));
        return -1;
    }

    EthBufDecoder d(frame);
    d.DecodeBuf( (unsigned char*)&head, sizeof(head) );
    // first 4 bytes after ethernet head must be 'OPVX'
    char *f = _ETH_SEND_HEAD_FLAG;
    char *p = head.flag;
    if( !(p[0]==f[0] && p[1]==f[1] && p[2]==f[2] && p[3]==f[3]) ) {
        esvc_log(ESVC_ERR" tag field \'%c%c%c%c\' != %s\n", p[0], p[1], p[2], p[3], _ETH_SEND_HEAD_FLAG);
        return -2;
    }

    size = ntohs(head.size);
    /*if(size != (frame.GetDataSize() - sizeof(head))) {
        esvc_log(ESVC_ERR" size field %d != actual size %d\n", size, frame.GetFrameSize() - sizeof(head) );
        return -3;
    }*/

    return 0;
}

void RawEthServer::dump_packet_svc(RawEtherFrame& frame)
{
    unsigned char* p;
    int i;
    int body_size;
    
    esvc_log("<<Service Packet dump:\n");
    esvc_log("<<SrcMAC [");
    p = frame.GetSrcMac().GetMac();
    for(i=0; i<6; i++) {
        esvc_log("%02x", p[i]);
        if(i!=5)
            esvc_log(":");
    }
    esvc_log("]\n");
    
    esvc_log("<<DstMAC [");
    p = frame.GetDestMac().GetMac();
    for(i=0; i<6; i++) {
        esvc_log("%02x", p[i]);
        if(i!=5)
            esvc_log(":");
    }
    esvc_log("]\n");

    esvc_log("<<Type [0x%04x]\n", frame.GetType()&0xffff );

    esvc_log("<<Tag [");
    struct RawSvcHead head;
    EthBufDecoder d(frame);
    d.DecodeBuf( (unsigned char*)&head, sizeof(head) );

    for(i=0; i<4; i++)
        esvc_log("%c", head.flag[i] & 0xff);
    esvc_log("]\n");

    body_size = ntohs(head.size);
    printf("<<Cmd Body Size [%d]\n", body_size);

    printf("<<uuid [");
    for(i=0; i<16; i++) {
        esvc_log("%02x", head.unique_id[i] & 0xff);
        if(i!=15)
            esvc_log("-");
    }
    esvc_log("]\n");

    char buf[ETH_DATA_LEN];
    d.DecodeBuf((unsigned char*)buf, body_size);

    printf("<<Service[%s]\n", buf);
    printf("<<dump end\n");
}

TClientConnVector::iterator RawEthServer::UpdateClient(RawEtherFrame& frame)
{
    int i;
    time_t tm;
    int client_cnt;
    int client_id = -1;
    
    struct RawSvcHead head;
    EthBufDecoder d(frame);
    d.DecodeBuf( (unsigned char*)&head, sizeof(head) );

    tm = time(NULL);
    TClientConnVector::iterator iter = _clients.begin();

    // remove dead client from list.
    while( iter != _clients.end() ) {
        if( difftime(tm, iter->_lastActive) > CLIENT_LIVE_TIME ) {
            if(_debug)
                esvc_log(ESVC_DBG "Client %s removed\n", iter->_mac.AsString().c_str());
            _clients.erase(iter);
            iter = _clients.begin();
        } else
            iter++;
    }
    
    // check if this client already in the list
    for(iter = _clients.begin(); iter != _clients.end(); iter++) {
        EtherMac mac = frame.GetSrcMac();
        if( iter->_mac.isEqual( mac ) ) {
            break;  
        }
    }

    if(iter == _clients.end() ) { // client not exist, add new.
        ClientConnection cc;
        if(_debug)
            esvc_log(ESVC_DBG "Client %s not exist\n", frame.GetSrcMac().AsString().c_str());

        cc._mac = frame.GetSrcMac();
        memcpy( cc._unique_id, head.unique_id, sizeof(head.unique_id));
        cc._lastActive = tm;
        _clients.push_back(cc);
        return _clients.end() - 1;
    } else {                        // client already exist, update info.
        if(_debug)
            esvc_log(ESVC_DBG "Client %s found\n", iter->_mac.AsString().c_str());

        if( memcmp(iter->_unique_id, head.unique_id, sizeof(head.unique_id)) == 0) {// duplicate packet, discard
            if(_debug)
                esvc_log(ESVC_DBG "Duplicate packet from %s\n", iter->_mac.AsString().c_str());
            return _clients.end();
        } else {      
            iter->_lastActive = tm;
            memcpy( iter->_unique_id, head.unique_id, sizeof(head.unique_id));
            return iter;
        }
    }
}

int RawEthServer::inBlackList(unsigned char* buf)
{
    int buflen = strlen((const char*)buf);
    int blacklen = _blackList.length();
    int i, j;

    for(i=0; i<buflen; i++) {
        for(j=0; j<blacklen; j++) {
            if( buf[i] == _blackList[j] ) {
                if(_debug)
                    esvc_log(ESVC_DBG "Char %c in blacklist\n", buf[i]);
                return -1;
            }
        }
    }
    
    return 0;
}

int RawEthServer::exec_cmd(string& cmd, string& result) 
{
    FILE * fp;
    int readsize;
    char buf[100+1];
    int ret;

    fp=popen( cmd.c_str(), "r");
    ret = 0;
    while(1){
        memset(buf, 0, sizeof(buf));
        readsize = fread(buf, 1, 100, fp);
        result += buf;
        ret += readsize;
        if(readsize!= 100 )
            break;
    };
    pclose(fp);
    return ret;
}

int RawEthServer::ProcessPacket(RawEtherFrame& frame)
{
    int i;
    int errno = 0;

    i = isValidPacket(frame);
    if( i ) { /* not a valid format */
        esvc_log(ESVC_ERR" not a valid service packet (%d)\n", i);
        return -1;
    }

    if(_dump)
        dump_packet_svc(frame);

    EtherMac test_mac = _txSocket.GetMac();
    if(frame.GetSrcMac().isEqual(test_mac)) {   // my own broadcast, omit
        if(_debug)
            esvc_log(ESVC_DBG " Boradcast from myself \n");
        return 0;
    }


    if( !frame.GetDestMac().isBroadCast() ) {
        if(!frame.GetDestMac().isEqual(test_mac)) {
            if(_debug)
                esvc_log(ESVC_ERR "dstmac <%s> not equ to my mac<%s>\n", (const char*)frame.GetDestMac().AsString().c_str(), (const char*)test_mac.AsString().c_str());
            return 0;
        }
    }

    TClientConnVector::iterator iter = UpdateClient(frame);
    if( iter == _clients.end() ) // duplicated packet, simply discard
        return -2;
    
    if( frame.GetDestMac().isBroadCast()) { // broadcast msg & not from myself i.e server ping.
        struct RawSvcHead head;
        EthBufDecoder d(frame);
        d.DecodeBuf( (unsigned char*)&head, sizeof(head) );
        unsigned char buf[ETH_DATA_LEN];
        int size = sizeof(buf);
        
        i = d.DecodeCStr(buf, size);
        if(i == 0) { // no string in buffer
            if(_debug)
                esvc_log(ESVC_DBG " Invalid boradcast packet format <%s>\n", frame.GetSrcMac().AsString().c_str());
            return -3;
        }

        if(strcmp((const char*)buf, _ETH_SVC_PING) == 0) { // gwping
            if(_debug)
                esvc_log(ESVC_DBG"Event <%s>\n", _ETH_SVC_PING);
            
            RawEtherFrame newFrame;
            EthBufCoder cd(newFrame);
            struct RawSvcHead txhead;
            txhead = head;  // copy from rx head.
            memcpy(txhead.flag, _ETH_RECV_HEAD_FLAG, strlen(_ETH_RECV_HEAD_FLAG));  // change to reply flag
            txhead.size = 0;     // must be change later

            cd.EncodeBuf((unsigned char*)&txhead, sizeof(txhead));
            //cd.EncodeCStr((unsigned char*)_ETH_SVC_PING);   // sender event's type

            TServerInfoVector::iterator iter = _serverInfoList.begin();
            string si;
            /* exexute commands one by one then send back to client */
            for(iter = _serverInfoList.begin(); iter != _serverInfoList.end(); iter++) {
                string r;
                if( iter->_isCmd ) {
                    exec_cmd(iter->_value, r);
                    r = iter->_key + "=" + r;
                } else 
                    r = iter->_key + "=" + iter->_value;
                
                i = cd.EncodeCStr( (unsigned char*)r.c_str() );
                if(i != (r.length()+1)) {
                    esvc_log(ESVC_WARN "cmd <%s> Encode error %d != %d\n", _ETH_SVC_PING, i, r.length()+1);
                }
            }

            cd.SetWord( (unsigned long)(&txhead.size) - (unsigned long)(&txhead), cd.GetDataSize() - sizeof(txhead) );

            newFrame.SetDataSize(cd.GetDataSize());
            newFrame.SetType(_ETHER_TYPE_OPVX);
            EtherMac mac = frame.GetSrcMac();
            newFrame.SetDestMac( mac);
            _txSocket.Send(newFrame);
        }
    } else {
        string exec_str;
        string stmp;
        TStringVector::iterator cl_iter;

        if(_debug)
            esvc_log(ESVC_DBG "Not broadcast\n");
        if(_dump)
            dump_packet_svc(frame);

        struct RawSvcHead head;
        EthBufDecoder d(frame);
        d.DecodeBuf( (unsigned char*)&head, sizeof(head) );
        unsigned char buf[ETH_DATA_LEN];
        int size = sizeof(buf);
        
        memset(buf, 0, sizeof(buf));
        i = d.DecodeCStr(buf, size);
        if(i == 0) { // no string in buffer
            if(_debug)
                esvc_log(ESVC_DBG " Invalid format from<%s>\n", frame.GetSrcMac().AsString().c_str());
            return -3;
        }

        stmp = (char*)buf;

        if(inBlackList(buf) == -1) {
            if(_debug)
                esvc_log(ESVC_DBG "Cmd have invalid char!<%s>\n", frame.GetSrcMac().AsString().c_str());
            errno = -4;
            exec_str = "EBLACKLIST";
            goto _RetValue;

        } else {
            if(_debug)
                esvc_log(ESVC_DBG "Cmd validation passed <%s>\n", frame.GetSrcMac().AsString().c_str());
        }

        for(cl_iter = _cmdList.begin(); cl_iter!=_cmdList.end(); cl_iter++) {
            if(  memcmp( cl_iter->c_str(), buf, cl_iter->length()) == 0 ) {  // valid cmd
                exec_cmd(stmp, exec_str);
                break;    
            }
        }

_RetValue:
		//Freedom delete 2013-10-18 11:05
        /*if(exec_str.length() == 0) {
            exec_str = "EINVALID";
            if(_debug)
                esvc_log(ESVC_ERR "Result of cmd <%s> is empty!\n", exec_str.c_str());
        }*/

        {
            RawEtherFrame newFrame;
            EthBufCoder cd(newFrame);
            struct RawSvcHead txhead;
            txhead = head;  // copy from rx head.
            memcpy(txhead.flag, _ETH_RECV_HEAD_FLAG, strlen(_ETH_RECV_HEAD_FLAG));  // change to reply flag
            txhead.size = 0;     // must be change later

            cd.EncodeBuf((unsigned char*)&txhead, sizeof(txhead));
            if(exec_str.length() > cd.GetFreeSize()) {  /* truncate the result */
                char* tail = "\n...\n";
                exec_str.erase(cd.GetFreeSize() - strlen(tail) -1, string::npos );
                exec_str += tail;
                if(_debug)
                    esvc_log(ESVC_WARN "cmd <%s> result tranced\n", buf);
            }
            i = cd.EncodeCStr( (unsigned char*)exec_str.c_str() );
            if(i != (exec_str.length()+1)) {
                if(_debug)
                    esvc_log(ESVC_WARN "cmd <%s> Encode error %d != %d\n", buf, i, exec_str.length()+1);
            }

            cd.SetWord( (unsigned long)(&txhead.size) - (unsigned long)(&txhead), cd.GetDataSize() - sizeof(txhead) );

            newFrame.SetDataSize(cd.GetDataSize());
            newFrame.SetType(_ETHER_TYPE_OPVX);
            EtherMac mac = frame.GetSrcMac();
            newFrame.SetDestMac( mac);
            _txSocket.Send(newFrame);
            if(_debug)
                esvc_log(ESVC_INFO "Send result of cmd <%s>\n", exec_str.c_str());
        }
    }

    return errno;
}

int main(int argc, char *argv[])
{
    int ret, i;
    short eth_p = _ETHER_TYPE_OPVX;
    RawEthServer server;
    char* ifName = NULL;
    char* iniName = NULL;

    if(argc <=2) {
        printf("Usage:\n");
        printf("\teth_server <ethx> <configfile>\n");
        printf("Where: \n\t<ethx> is the name of ethernet interface used.\n");
        printf("\t<configfile> is the name of config file.\n");
        printf("Sample:\n\teth_server eth0 /etc/eth_server.ini\n\n");
        return 0;
    }

    ifName = argv[1];
    iniName = argv[2];

    ret = server.init(ifName, iniName);
    if(ret<0) {
        esvc_log(ESVC_ERR "server.init return %d\n", ret);
        return ret;
    }

    /* loop to process request */
    while(1){
        int numbytes;
        RawEtherFrame frame;
        
        numbytes = 0;
        esvc_log(ESVC_INFO "Waiting to recvfrom %d bytes...\n", frame.GetMaxDataSize());
        numbytes = server.Recv(frame);
    
        if(numbytes==-1) {
            esvc_log(ESVC_INFO "recvfrom error -1\n");
            exit(1);
        }

        server.ProcessPacket(frame);
    }

    return ret;
}


