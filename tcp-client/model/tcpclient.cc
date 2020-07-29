#include"tcpclient.h"
#include "ns3/log.h"
#include <string>
namespace ns3
{
NS_LOG_COMPONENT_DEFINE("TCPClient");
const std::string log_name("tcp_");
TcpClient::TcpClient(uint32_t client_id)
{
    m_cid=client_id;
	m_currentTxBytes=0;
    m_maxBytes=0;
    m_sendSize=1000;
}
void TcpClient::SetCongestionAlgo(std::string algo){
    m_algo=algo;
}
void TcpClient::ConfigureCongstionAlgo()
{
	TypeId id;
	if(m_algo.compare("TcpNewReno")==0)
	{
		id=(TcpNewReno::GetTypeId ());
	}else if(m_algo.compare("TcpVegas")==0)
	{
		id=(TcpVegas::GetTypeId ());
	}else if(m_algo.compare("TcpBic")==0)
	{
		id=TcpBic::GetTypeId ();
	}else if (m_algo.compare ("TcpWestwood") == 0)
	{
		id=TcpWestwood::GetTypeId ();
	}
	else{
		NS_LOG_INFO("default congestion algorithm");
		return ;
	}
	ObjectFactory congestionAlgorithmFactory;
	congestionAlgorithmFactory.SetTypeId (id);
	Ptr<TcpCongestionOps> algo = congestionAlgorithmFactory.Create<TcpCongestionOps> ();
	TcpSocketBase *base=static_cast<TcpSocketBase*>(PeekPointer(m_socket));	
	base->SetCongestionControlAlgorithm (algo);
}
void TcpClient::SetMaxBytes (uint64_t maxBytes){
    m_maxBytes=maxBytes;
}
void TcpClient::ConfigurePeer(Ipv4Address addr,uint16_t port){
    m_servAddr=addr;
    m_servPort=port;
}
void TcpClient::StartApplication (void){
    m_socket=Socket::CreateSocket(GetNode(),TcpSocketFactory::GetTypeId ());
    m_socket->Bind ();
    m_socket->SetAttribute("SegmentSize",UintegerValue(1400));
    ConfigureCongstionAlgo();
    
    if(m_log|E_TCP_CWND){
        m_trace.OpenCwndTraceFile(log_name+std::to_string(m_cid));
        m_socket->TraceConnectWithoutContext ("CongestionWindow", MakeCallback(&TcpTracer::OnCwnd,&m_trace));
    }
    if(m_log|E_TCP_RTT){
        m_trace.OpenRttTraceFile(log_name+std::to_string(m_cid));
        m_socket->TraceConnectWithoutContext ("RTT",MakeCallback(&TcpTracer::OnRtt,&m_trace));
    }
    if(m_log|E_TCP_BW){
        //m_trace.OpenBandwidthTraceFile(log_name+std::to_string(m_cid));
        //m_socket->TraceConnectWithoutContext ("Bandwidth",MakeCallback(&TcpTracer::OnBandwidth,&m_trace));        
    }
    m_socket->Connect (InetSocketAddress (m_servAddr, m_servPort));
    m_socket->ShutdownRecv ();
    m_socket->SetConnectCallback (
    MakeCallback (&TcpClient::ConnectionSucceeded, this),
    MakeCallback (&TcpClient::ConnectionFailed, this));
    m_socket->SetSendCallback (
    MakeCallback (&TcpClient::DataSend, this));
}
void TcpClient::StopApplication (void){
    if (m_socket != 0)
    {
        m_socket->Close ();
        m_connected = false;
    }    
}
void TcpClient::ConnectionSucceeded (Ptr<Socket> socket)
{
  NS_LOG_INFO("Connection succeeded");
  m_connected=true;
  SendData ();
}
void TcpClient::ConnectionFailed(Ptr<Socket>Socket)
{
    
}
void TcpClient::DataSend (Ptr<Socket> socket, uint32_t){
    if(m_connected){
        SendData();
    }
}

static uint64_t  Min(uint64_t left,uint64_t right)
{
	uint64_t min=left<right?left:right;
	return min;
}
void TcpClient::SendData()
{
 while (m_currentTxBytes <m_maxBytes&&m_socket->GetTxAvailable () > 0) 
    {
      uint64_t left =m_maxBytes-m_currentTxBytes;
      uint64_t toWrite = Min (m_sendSize, left);
      toWrite = Min(toWrite, m_socket->GetTxAvailable ());
      uint64_t amountSent=0;
      if(toWrite>0){
        amountSent =m_socket->Send (0, toWrite, 0);//means,no data;
        if(amountSent < 0)
        {
          // we will be called again when new tx space becomes available.
          return;
        }          
      }
      m_currentTxBytes += amountSent;
    }	
}

}
