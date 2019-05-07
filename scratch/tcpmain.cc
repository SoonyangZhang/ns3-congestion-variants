#include <iostream>
#include <fstream>
#include <string>
#include<stdio.h>
#include<unistd.h>
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
static const uint32_t totalTxBytes = 2000000000;
// Perform series of 1040 byte writes (this is a multiple of 26 since
// we want to detect data splicing in the output stream)
static const uint32_t writeSize = 1040;
static const double simDuration    = 30.0;
uint8_t data[writeSize];
class Tracer
{
public:
	Tracer(){}
	~Tracer()
	{
	if (m_TcpWindowFile.is_open())
		m_TcpWindowFile.close();
	if (m_rttTraceFile.is_open())
		m_rttTraceFile.close();
	}
void CwndTracer (uint32_t oldval, uint32_t newval)
{
	TcpWindowTrace(oldval,newval);
	NS_LOG_INFO (Simulator::Now().GetSeconds()<<"Moving cwnd from " << oldval << " to " << newval);
}
void OpenCwndTraceFile(std::string filename)
{

	char buf[FILENAME_MAX];
	string path = string (getcwd(buf, FILENAME_MAX)) + "/traces/" + filename;
	m_TcpWindowFile.open(path.c_str(), fstream::out);

	sprintf (buf,  "%16s %16s",
			"Time", "Window");

	NS_ASSERT_MSG (m_TcpWindowFile.is_open(), "No trace file to write to");
	m_TcpWindowFile << buf << std::endl;
}
void CloseTcpWindowTraceFile()
{
	if (m_TcpWindowFile.is_open())
		m_TcpWindowFile.close();
	else
		NS_LOG_ERROR("Open file not found");
}
void TcpWindowTrace(uint32_t oldval, uint32_t newval)
{
	char line [255];
	oldval=oldval;
	sprintf (line, "%16f %16f",
			Simulator::Now().GetSeconds(),(double)newval/1460.0);

	NS_ASSERT_MSG (m_TcpWindowFile.is_open(), "No trace file to write to");
	m_TcpWindowFile << line << std::endl;
}
void OpenRttTraceFile(std::string filename)
{
	char buf[FILENAME_MAX];
	string path = string (getcwd(buf, FILENAME_MAX)) + "/traces/" + filename;
	m_rttTraceFile.open(path.c_str(), fstream::out);
	//sprintf (buf,  "%16s %16s","Time", "rtt");
	NS_ASSERT_MSG (m_rttTraceFile.is_open(), "No trace file to write to");
	//m_rttTraceFile << buf << std::endl;
}
void CloseRttTraceFile()
{
	if (m_rttTraceFile.is_open())
		m_rttTraceFile.close();
	else
		NS_LOG_ERROR("Open file not found");	
}
void RttTracer(Time oldval, Time newval)
{
	char line [255];
	oldval=oldval;
	sprintf (line, "%16f %16f",
			Simulator::Now().GetSeconds(),newval.GetSeconds ());

	NS_ASSERT_MSG (m_rttTraceFile.is_open(), "No trace file to write to");
	m_rttTraceFile<< line << std::endl;	
}
private:
fstream m_TcpWindowFile;
fstream m_rttTraceFile;
};
void RxDrop(Ptr<const Packet>p)
{
	Ptr<Packet> packet=p->Copy(); 
        PppHeader ppp;
          packet->RemoveHeader(ppp);
         Ipv4Header ipHeader;
         packet->RemoveHeader (ipHeader);
        if(packet->GetSize()>500)
        {
          TcpHeader header;
	packet->RemoveHeader(header);
	NS_LOG_INFO(Simulator::Now().GetSeconds()<<" drop packet "<<header.GetSequenceNumber()<<" source port "<<header.GetSourcePort ()<<" dest port "<<header.GetDestinationPort ());              
        }

        //NS_LOG_INFO("size "<<packet->GetSize()<<"source "<<ipHeader.GetSource()<<"dest "<<ipHeader.GetDestination());

}
int main(int argc, char *argv[])
{
	LogComponentEnable("TCPMain", LOG_LEVEL_ALL);	
    	//LogComponentEnable("TCPClient", LOG_LEVEL_ALL);
	uint32_t queueSize=25;
	CommandLine cmd;
	cmd.Parse (argc, argv);

	// initialize the tx buffer.
	for(uint32_t i = 0; i < writeSize; ++i)
    {
      char m = toascii (97 + i % 26);
      data[i] = m;
    }
	// Set a few attributes
	Config::SetDefault ("ns3::RateErrorModel::ErrorRate", DoubleValue (0.01));
	Config::SetDefault ("ns3::RateErrorModel::ErrorUnit", StringValue ("ERROR_UNIT_PACKET"));

	Config::SetDefault ("ns3::BurstErrorModel::ErrorRate", DoubleValue (0.05));
	Config::SetDefault ("ns3::BurstErrorModel::BurstSize", StringValue ("ns3::UniformRandomVariable[Min=1|Max=3]"));
	std::string errorModelType = "ns3::RateErrorModel";	
	NodeContainer n0n1;
	n0n1.Create (2);

	NodeContainer n1n2;
	n1n2.Add (n0n1.Get (1));
	n1n2.Create (1);
	InternetStackHelper stack;
	stack.Install (n0n1);
	stack.Install (n1n2.Get(1));
	
	PointToPointHelper p2p;
	p2p.SetDeviceAttribute ("DataRate", StringValue("2Mbps"));
	p2p.SetChannelAttribute ("Delay", StringValue("25ms"));
	p2p.SetQueue ("ns3::DropTailQueue", "MaxPackets",UintegerValue(queueSize),"Mode",EnumValue (Queue::QueueMode::QUEUE_MODE_PACKETS));	
	NetDeviceContainer dev0 = p2p.Install (n0n1);
	NetDeviceContainer dev1 = p2p.Install (n1n2);
	Ipv4AddressHelper ipv4;
	ipv4.SetBase ("10.1.3.0", "255.255.255.0");
	ipv4.Assign (dev0);
	ipv4.SetBase ("10.1.2.0", "255.255.255.0");
	Ipv4InterfaceContainer ipInterfs = ipv4.Assign (dev1);
	TrafficControlHelper tch;
	tch.Uninstall (dev0);
	tch.Uninstall (dev1);
	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  	ObjectFactory factory;
  	factory.SetTypeId (errorModelType);
  	Ptr<ErrorModel> em = factory.Create<ErrorModel> ();
  	dev1.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
  	dev1.Get (1)->TraceConnectWithoutContext("PhyRxDrop", MakeCallback (&RxDrop));

	uint16_t servPort = 50000;

	// Create a packet sink to receive these packets on n2...
	PacketSinkHelper sink ("ns3::TcpSocketFactory",
                         InetSocketAddress (Ipv4Address::GetAny (), servPort));

	ApplicationContainer apps = sink.Install (n1n2.Get (1));
	apps.Start (Seconds (0.0));
	apps.Stop (Seconds (simDuration));
	TcpClient client1(1);
	client1.CreateSocket(n0n1.Get (0));
	
	Tracer tracer1;
	std::string cwndtracefile1="cwnd1.tr";
	tracer1.OpenCwndTraceFile(cwndtracefile1);
	std::string rtttracefile1="rtt1.tr";
	tracer1.OpenRttTraceFile(rtttracefile1);
	client1.GetSocket()->TraceConnectWithoutContext ("CongestionWindow", MakeCallback(&Tracer::CwndTracer,&tracer1));
	client1.GetSocket()->TraceConnectWithoutContext ("RTT",MakeCallback(&Tracer::RttTracer,&tracer1));
	client1.SetWriteData(data,writeSize,totalTxBytes);
	client1.StopTime(simDuration);
	client1.StartFlow(ipInterfs.GetAddress (1), servPort);

	
	TcpClient client2(2);
	client2.CreateSocket(n0n1.Get (0));
	//client2.SetCongestionAlgo(string("TcpBic"));
	Tracer tracer2;
	std::string cwndtracefile2="cwnd2.tr";
	tracer2.OpenCwndTraceFile(cwndtracefile2);
	client2.GetSocket()->TraceConnectWithoutContext ("CongestionWindow", MakeCallback(&Tracer::CwndTracer,&tracer2));
	
	client2.SetWriteData(data,writeSize,totalTxBytes);
	client2.StopTime(simDuration);
	client2.StartFlow(ipInterfs.GetAddress (1), servPort);

	TcpClient client3(3);
	client3.CreateSocket(n0n1.Get (0));
	//client3.SetCongestionAlgo(string("TcpWestwood"));
	Tracer tracer3;
	std::string cwndtracefile3="cwnd3.tr";
	tracer3.OpenCwndTraceFile(cwndtracefile3);
	client3.GetSocket()->TraceConnectWithoutContext ("CongestionWindow", MakeCallback(&Tracer::CwndTracer,&tracer3));
	client3.SetWriteData(data,writeSize,totalTxBytes);
	client3.StopTime(simDuration);
	client3.StartFlow(ipInterfs.GetAddress (1), servPort);
	Simulator::Stop (Seconds (simDuration+10.0));
	Simulator::Run ();
	Simulator::Destroy ();
	return 0;
	
}
