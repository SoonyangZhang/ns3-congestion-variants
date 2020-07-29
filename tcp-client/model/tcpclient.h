#pragma once
#include<string>
#include "ns3/address.h"
#include "ns3/application.h"
#include "ns3/internet-module.h"
#include "ns3/tcptracer.h"
namespace ns3
{
class TcpClient:public Application
{
public:
    enum TcpClientTraceEnable:uint8_t{
        E_TCP_CWND=0x01,
        E_TCP_RTT=0x02,
        E_POSSION_ALL=E_TCP_CWND|E_TCP_RTT,
    };
    TcpClient(uint32_t client_id);
    void SetCongestionAlgo(std::string algo);
    void SetMaxBytes (uint64_t maxBytes);
    void ConfigurePeer(Ipv4Address addr,uint16_t port);
    void EnableTrace(uint8_t log){
        m_log=log;
    }
    Ptr<Socket> GetSocket() const {return m_socket;}
private:
    virtual void StartApplication (void);
    virtual void StopApplication (void);
    void ConfigureCongstionAlgo();
    void ConnectionSucceeded (Ptr<Socket> socket);
    void ConnectionFailed (Ptr<Socket> socket);
    void DataSend (Ptr<Socket>, uint32_t); // for socket's SetSendCallback
    void SendData();
    uint32_t m_cid;
    uint64_t m_maxBytes;
    uint64_t m_currentTxBytes;
    uint32_t m_sendSize;
    bool m_connected{false};
    std::string m_algo{"TcpNewReno"};
    Ipv4Address m_servAddr;
    uint16_t m_servPort;
    Ptr<Socket> m_socket;
    uint8_t m_log{0};
    TcpTracer m_trace;
};	
}

