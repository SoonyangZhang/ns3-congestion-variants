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
        E_TCP_BW=0x04,
        E_TCP_ALL=E_TCP_CWND|E_TCP_RTT|E_TCP_BW,
    };
    TcpClient(uint64_t bytes,uint8_t log=0);
    void ConfigurePeer(Address &addr);
    void SetCongestionAlgo(std::string &algo);
private:
    virtual void StartApplication (void);
    virtual void StopApplication (void);
    void ConfigureCongstionAlgo();
    void ConnectionSucceeded (Ptr<Socket> socket);
    void ConnectionFailed (Ptr<Socket> socket);
    void OnCanWrite(Ptr<Socket>, uint32_t); // for socket's SetSendCallback
    void NotifiSendData();
    uint32_t m_uuid=0;
    uint64_t m_targetBytes=0;
    uint64_t m_currentTxBytes=0;
    uint8_t m_log=0;
    bool m_connected=false;
    std::string m_algo{"TcpNewReno"};
    Address m_serverAddr_;
    Ptr<Socket> m_socket;
    TcpTracer m_trace;
};
}

