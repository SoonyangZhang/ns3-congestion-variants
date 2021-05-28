#include <string>
#include <algorithm>
#include"tcpclient.h"
#include "ns3/log.h"
namespace ns3{
const uint32_t kMSS=1400;
int kTcpUniqueCount=0;
NS_LOG_COMPONENT_DEFINE("tcp-client");
const std::string log_name("tcp_");
TcpClient::TcpClient(uint64_t bytes,uint8_t log){
    m_uuid=kTcpUniqueCount;
    kTcpUniqueCount++;
    m_targetBytes=bytes;
    m_log=log;
}
void TcpClient::SetCongestionAlgo(std::string &algo){
    m_algo=algo;
}
void TcpClient::ConfigureCongstionAlgo(){
    TypeId id;
    if(0==m_algo.compare("reno")){
        id=TcpNewReno::GetTypeId ();
    }else if(0==m_algo.compare("linux-reno")){
        id=TcpLinuxReno::GetTypeId();
    }else if(0==m_algo.compare("vegas")){
        id=TcpVegas::GetTypeId ();
    }else if(0==m_algo.compare("bic")){
        id=TcpBic::GetTypeId ();
    }else if(0==m_algo.compare("cubic")){
        id=TcpCubic::GetTypeId();
    }else if (0==m_algo.compare ("westwood")){
        id=TcpWestwood::GetTypeId ();
    }else{
        return ;
    }
    ObjectFactory congestionAlgorithmFactory;
    congestionAlgorithmFactory.SetTypeId (id);
    Ptr<TcpCongestionOps> algo = congestionAlgorithmFactory.Create<TcpCongestionOps> ();
    TcpSocketBase *base=static_cast<TcpSocketBase*>(PeekPointer(m_socket));	
    base->SetCongestionControlAlgorithm (algo);
}
void TcpClient::ConfigurePeer(Address &addr){
    m_serverAddr_=addr;
}
void TcpClient::StartApplication (void){
    m_socket=Socket::CreateSocket(GetNode(),TcpSocketFactory::GetTypeId ());
    m_socket->Bind ();
    m_socket->SetAttribute("SegmentSize",UintegerValue(kMSS));
    ConfigureCongstionAlgo();
    if(m_log&E_TCP_CWND){
        m_trace.OpenCwndTraceFile(log_name+std::to_string(m_uuid));
        m_socket->TraceConnectWithoutContext ("CongestionWindow", MakeCallback(&TcpTracer::OnCwnd,&m_trace));
    }
    if(m_log&E_TCP_RTT){
        m_trace.OpenRttTraceFile(log_name+std::to_string(m_uuid));
        m_socket->TraceConnectWithoutContext ("RTT",MakeCallback(&TcpTracer::OnRtt,&m_trace));
    }
    if(m_log&E_TCP_BW){
        //m_trace.OpenBandwidthTraceFile(log_name+std::to_string(m_uuid));
        //m_socket->TraceConnectWithoutContext ("Bandwidth",MakeCallback(&TcpTracer::OnBandwidth,&m_trace));        
    }
    m_socket->Connect(m_serverAddr_);
    m_socket->ShutdownRecv();
    m_socket->SetConnectCallback(MakeCallback (&TcpClient::ConnectionSucceeded, this),
                                    MakeCallback (&TcpClient::ConnectionFailed, this));
    m_socket->SetSendCallback(MakeCallback (&TcpClient::OnCanWrite, this));
}
void TcpClient::StopApplication (void){
    if (m_socket){
        m_socket->Close ();
        m_connected = false;
    }
}
void TcpClient::ConnectionSucceeded (Ptr<Socket> socket){
    NS_LOG_INFO("Connection succeeded");
    m_connected=true;
    NotifiSendData ();
}
void TcpClient::ConnectionFailed(Ptr<Socket>Socket){}
void TcpClient::OnCanWrite(Ptr<Socket> socket, uint32_t){
    if(m_connected){
        NotifiSendData();
    }
}
void TcpClient::NotifiSendData(){
    while (m_currentTxBytes <m_targetBytes&&m_socket->GetTxAvailable () > 0){
        uint64_t left =m_targetBytes-m_currentTxBytes;
        uint32_t to_write=std::min((uint64_t)kMSS, (uint64_t)left);
        to_write =std::min(to_write,m_socket->GetTxAvailable ());
        int ret=0;
        if(to_write>0){
            ret=m_socket->Send (0, to_write, 0);//means,no data;
            if(ret<0){
                // we will be called again when new tx space becomes available.
                return;
            }
        }
        m_currentTxBytes+=ret;
    }
}
}
