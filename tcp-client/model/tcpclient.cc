#include"tcpclient.h"
//in this  situation ,its not quit easy
namespace ns3
{
NS_LOG_COMPONENT_DEFINE("TCPClient");
static	void WriteUntilBufferFull1(Ptr<Socket>Socket,uint32_t txSpace);
static	void ConnectionSucceeded(Ptr<Socket>);
static	void ConnectionFailed(Ptr<Socket>);

static	void HandlePeerClose (Ptr<Socket>);
static	void HandlePeerError (Ptr<Socket>);
static	void CloseConnection (Ptr<Socket>);
TcpClient::TcpClient(uint32_t clientid)
{
	m_currentTxBytes=0;
	m_cid=clientid;
}
void TcpClient::CreateSocket(Ptr<Node> node)
{
	m_socket=Socket::CreateSocket(node,TcpSocketFactory::GetTypeId ());
	m_socket->Bind ();
	m_socket->SetAttribute("SegmentSize",UintegerValue(1460));
	m_socket->SetHandler((void*)this);
	m_socket->SetConnectCallback(MakeCallback(&ConnectionSucceeded),
	MakeCallback(&ConnectionFailed));
}	
void TcpClient::SetCongestionAlgo(std::string name)
{
	TypeId id;
	if(name.compare("TcpNewReno")==0)
	{
		id=(TcpNewReno::GetTypeId ());
	}else if(name.compare("TcpVegas")==0)
	{
		id=(TcpVegas::GetTypeId ());
	}else if(name.compare("TcpBic")==0)
	{
		id=TcpBic::GetTypeId ();
	}else if (name.compare ("TcpWestwood") == 0)
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
static void ConnectionSucceeded(Ptr<Socket>Socket)
{
    TcpClient *client=static_cast<TcpClient*>(Socket->GetHandler());
	NS_LOG_FUNCTION_NOARGS();
    NS_LOG_INFO(client->m_cid<<"Connection requeste succeed");
	Socket->SetDataSentCallback (MakeCallback (&WriteUntilBufferFull1));
    Simulator::ScheduleNow (&WriteUntilBufferFull1, Socket, 0);
    Simulator::Schedule (Seconds (client->m_duration), &CloseConnection, Socket);	
}
static void ConnectionFailed(Ptr<Socket>Socket)
{
    TcpClient *client=static_cast<TcpClient*>(Socket->GetHandler());
	NS_LOG_INFO(client->m_cid<<"Connection requeste failure");
    Socket->Close();	
}
static void HandlePeerClose (Ptr<Socket>Socket)
{
	TcpClient *client=static_cast<TcpClient*>(Socket->GetHandler());
	NS_LOG_INFO(client->m_cid<<"Connection closed by peer");
	Socket->Close();
}
static void HandlePeerError (Ptr<Socket>Socket)
{
	TcpClient *client=static_cast<TcpClient*>(Socket->GetHandler());
	NS_LOG_INFO(client->m_cid<<"Connection closed by peer error");
	Socket->Close();	
}
static void CloseConnection (Ptr<Socket> Socket)
{
	TcpClient *client=static_cast<TcpClient*>(Socket->GetHandler()); 
	Socket->Close();
    NS_LOG_LOGIC(client->m_cid<<"currentTxBytes = " <<client->m_currentTxBytes);
    NS_LOG_LOGIC(client->m_cid<<"totalTxBytes   = " <<client->m_total);
    NS_LOG_LOGIC(client->m_cid<<"connection to remote host has been closed");	
}
static uint64_t  Min(uint64_t left,uint64_t right)
{
	uint64_t min=left<right?left:right;
	return min;
}
static void WriteUntilBufferFull1(Ptr<Socket>Socket,uint32_t txSpace)
{
	TcpClient *client=static_cast<TcpClient*>(Socket->GetHandler());
 while (client->m_currentTxBytes < client->m_total&&Socket->GetTxAvailable () > 0) 
    {
      uint64_t left = client->m_total - client->m_currentTxBytes;
      uint64_t dataOffset = client->m_currentTxBytes % client->m_writeSize;
      uint64_t toWrite = client->m_writeSize - dataOffset;
      toWrite = Min (toWrite, left);
      toWrite = Min(toWrite, Socket->GetTxAvailable ());
      //int amountSent =Socket->Send (&client->m_data[dataOffset], toWrite, 0);
      int amountSent =Socket->Send (0, toWrite, 0);//means,no data;
      NS_LOG_INFO(client->m_cid<<"DATA SENT in WriteUntilBufferFull"<<amountSent);
      if(amountSent < 0)
        {
          // we will be called again when new tx space becomes available.
          return;
        }
      client->m_currentTxBytes += amountSent;
    }	
}
}
