#include <string>
#include<stdio.h>
#include <iostream>
#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/tcp-client-module.h"
using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("TCPMain");
// The number of bytes to send in this simulation.
static const uint32_t totalTxBytes = 20000*1500;
static const double startTime=0;
static const double simDuration    = 100.0;
int main(int argc, char *argv[])
{
	LogComponentEnable("TCPMain", LOG_LEVEL_ALL);	
    LogComponentEnable("TCPClient", LOG_LEVEL_ALL);
	CommandLine cmd;
	cmd.Parse (argc, argv);
	// Set a few attributes
	Config::SetDefault ("ns3::RateErrorModel::ErrorRate", DoubleValue (0.01));
	Config::SetDefault ("ns3::RateErrorModel::ErrorUnit", StringValue ("ERROR_UNIT_PACKET"));

	Config::SetDefault ("ns3::BurstErrorModel::ErrorRate", DoubleValue (0.05));
	Config::SetDefault ("ns3::BurstErrorModel::BurstSize", StringValue ("ns3::UniformRandomVariable[Min=1|Max=3]"));
	std::string errorModelType = "ns3::RateErrorModel";	
	NodeContainer n0n1;
	n0n1.Create (2);

	//NodeContainer n1n2;
	//n1n2.Add (n0n1.Get (1));
	//n1n2.Create (1);
	InternetStackHelper stack;
	stack.Install (n0n1);

	
    uint32_t bps=2000000;
    uint32_t msDelay=50;
    uint32_t msQdelay=100;
    uint32_t mtu=1500;
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute ("DataRate", DataRateValue  (DataRate (bps)));
    pointToPoint.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (msDelay)));
    auto bufSize = std::max<uint32_t> (mtu, bps * msQdelay / 8000);
    int packets=bufSize/mtu;
	pointToPoint.SetQueue ("ns3::DropTailQueue",
                  "MaxSize", StringValue (std::to_string(1)+"p"));
    NetDeviceContainer dev0 = pointToPoint.Install (n0n1);
    
    TrafficControlHelper pfifoHelper;
    uint16_t handle = pfifoHelper.SetRootQueueDisc ("ns3::FifoQueueDisc", "MaxSize", StringValue (std::to_string(packets)+"p"));
    pfifoHelper.AddInternalQueues (handle, 1, "ns3::DropTailQueue", "MaxSize",StringValue (std::to_string(packets)+"p"));
    pfifoHelper.Install(dev0);
    
    
	Ipv4AddressHelper ipv4Helper;
	ipv4Helper.SetBase ("10.1.1.0", "255.255.255.0");
	ipv4Helper.Assign (dev0);
	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
    
    bool error=false;
    if(error){
        ObjectFactory factory;
        factory.SetTypeId (errorModelType);
        Ptr<ErrorModel> em = factory.Create<ErrorModel> ();
        dev0.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
        //dev0.Get (1)->TraceConnectWithoutContext("PhyRxDrop", MakeCallback (&RxDrop));        
    }

    
    uint16_t servPort = 5000;
    uint32_t client_id=1;
    PacketSinkHelper sink ("ns3::TcpSocketFactory",
                        InetSocketAddress (Ipv4Address::GetAny (), servPort));
    
    Ptr<Node> serverNode=n0n1.Get (1);
    Ptr<Ipv4> ipv4 = serverNode->GetObject<Ipv4> ();
	Ipv4Address servIp = ipv4->GetAddress (1, 0).GetLocal();
    ApplicationContainer apps = sink.Install (serverNode);
    apps.Start (Seconds (0.0));
    apps.Stop (Seconds (simDuration));
    
    
    Ptr<TcpClient>  client1= CreateObject<TcpClient> (client_id);
    n0n1.Get(0)->AddApplication(client1);
    client1->SetMaxBytes(totalTxBytes);
    client1->ConfigurePeer(servIp,servPort);
    client1->EnableTrace(TcpClient::E_TCP_CWND|TcpClient::E_TCP_RTT|TcpClient::E_TCP_BW);
    client1->SetStartTime (Seconds (startTime));
    client1->SetStopTime (Seconds (simDuration));
    
    client_id++;
    
    Simulator::Stop (Seconds (simDuration+10.0));
    Simulator::Run ();
    Simulator::Destroy ();
    return 0;
    
}
