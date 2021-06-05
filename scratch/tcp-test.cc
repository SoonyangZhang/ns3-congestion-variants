#include <string>
#include <stdio.h>
#include <unistd.h>
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

NS_LOG_COMPONENT_DEFINE ("TcpTest");
static const double startTime=0;
static const double simDuration= 200.0;
#define DEFAULT_PACKET_SIZE 1500
static NodeContainer BuildExampleTopo (uint64_t bps,
                                       uint32_t msDelay,
                                       uint32_t msQdelay,
                                       bool enable_random_loss=false)
{
    NodeContainer nodes;
    nodes.Create (2);

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute ("DataRate", DataRateValue  (DataRate (bps)));
    pointToPoint.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (msDelay)));
    auto bufSize = std::max<uint32_t> (DEFAULT_PACKET_SIZE, bps * msQdelay / 8000);
    int packets=bufSize/DEFAULT_PACKET_SIZE;
    pointToPoint.SetQueue ("ns3::DropTailQueue",
                           "MaxSize", StringValue (std::to_string(1)+"p"));
    NetDeviceContainer devices = pointToPoint.Install (nodes);

    InternetStackHelper stack;
    stack.Install (nodes);

    TrafficControlHelper pfifoHelper;
    uint16_t handle = pfifoHelper.SetRootQueueDisc ("ns3::FifoQueueDisc", "MaxSize", StringValue (std::to_string(packets)+"p"));
    pfifoHelper.AddInternalQueues (handle, 1, "ns3::DropTailQueue", "MaxSize",StringValue (std::to_string(packets)+"p"));
    pfifoHelper.Install(devices);
    Ipv4AddressHelper address;
    std::string nodeip="10.1.1.0";
    address.SetBase (nodeip.c_str(), "255.255.255.0");
    address.Assign (devices);
    if(enable_random_loss){
        std::string errorModelType = "ns3::RateErrorModel";
        ObjectFactory factory;
        factory.SetTypeId (errorModelType);
        Ptr<ErrorModel> em = factory.Create<ErrorModel> ();
        devices.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
    }
    return nodes;
}
// ./waf --run "scratch/tcp-test --cong reno"
int main(int argc, char *argv[])
{
    LogComponentEnable("TcpTest", LOG_LEVEL_ALL);
    LogComponentEnable("TcpClient", LOG_LEVEL_ALL);
    LogComponentEnable("TcpServer", LOG_LEVEL_ALL);
    LogComponentEnable("TcpSink", LOG_LEVEL_ALL);
    uint32_t kMaxmiumSegmentSize=1400;
    Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(200*kMaxmiumSegmentSize));
    Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(200*kMaxmiumSegmentSize));
    Config::SetDefault("ns3::TcpSocket::SegmentSize",UintegerValue(kMaxmiumSegmentSize));
    uint32_t link_bw=6000000;
    uint32_t link_owd=100;
    uint32_t q_delay=300;
    std::string cong_arg("linux-reno");
    CommandLine cmd;
    cmd.AddValue ("cong", "congestion algorithm",cong_arg);
    cmd.Parse (argc, argv);

    NodeContainer topo;
    topo=BuildExampleTopo(link_bw,link_owd,q_delay);
    Ptr<Node> h1=topo.Get(0);
    Ptr<Node> h2=topo.Get(1);

    uint16_t serv_port = 5000;
    Ptr<Ipv4> ipv4 = h2->GetObject<Ipv4> ();
    Ipv4Address serv_ip = ipv4->GetAddress (1, 0).GetLocal();
    Address dummy_addr;
    {
        InetSocketAddress socket_addr=InetSocketAddress{serv_ip,serv_port};
        dummy_addr=socket_addr;
        PacketSinkHelper sink ("ns3::TcpSocketFactory",socket_addr);
        ApplicationContainer apps = sink.Install (h2);
        apps.Start (Seconds (0.0));
        apps.Stop (Seconds (simDuration));
        serv_port++;
    }
    Address tcp_sink_addr;
    {
        InetSocketAddress socket_addr=InetSocketAddress{serv_ip,serv_port};
        tcp_sink_addr=socket_addr;
        Ptr<TcpServer> server=CreateObject<TcpServer>(tcp_sink_addr);
        h2->AddApplication(server);
        server->SetStartTime (Seconds (0.0));
        serv_port++;
    }

    uint64_t totalTxBytes = 20000*1500;
    std::string cong_algo("linux-reno");
    if(0==cong_arg.compare("bic")){
        cong_algo=cong_arg;
    }else if(0==cong_arg.compare("cubic")){
        cong_algo=cong_arg;
    }else if(0==cong_arg.compare("reno")){
        cong_algo=cong_arg;
    }else if(0==cong_arg.compare("linux-reno")){
        cong_algo=cong_arg;
    }else if(0==cong_arg.compare("westwood")){
        cong_algo=cong_arg;
    }
    //create folder to collect trace
    {
        char buf[FILENAME_MAX];
        memset(buf,0,FILENAME_MAX);
        std::string result_folder=cong_algo;
        std::string parent=std::string (getcwd(buf, FILENAME_MAX));
        std::string trace_folder=parent+ "/traces/"+result_folder;
        MakePath(trace_folder);
        TcpTracer::SetTraceFolder(trace_folder.c_str());
    }

    //with trace
    //first tcp client
    {
        Ptr<TcpClient>  client= CreateObject<TcpClient> (totalTxBytes,TcpClient::E_TRACE_ALL);
        h1->AddApplication(client);
        client->ConfigurePeer(tcp_sink_addr);
        client->SetCongestionAlgo(cong_algo);
        client->SetStartTime (Seconds (startTime));
        client->SetStopTime (Seconds (simDuration));
    }
    //second tcp client
    {
        Ptr<TcpClient>  client= CreateObject<TcpClient> (totalTxBytes,TcpClient::E_TRACE_ALL);
        h1->AddApplication(client);
        client->ConfigurePeer(tcp_sink_addr);
        client->SetCongestionAlgo(cong_algo);
        client->SetStartTime (Seconds (startTime));
        client->SetStopTime (Seconds (simDuration));
    }
    //without trace
    //third tcp client
    {
        Ptr<TcpClient>  client= CreateObject<TcpClient> (totalTxBytes);
        h1->AddApplication(client);
        client->ConfigurePeer(dummy_addr);
        client->SetCongestionAlgo(cong_algo);
        client->SetStartTime (Seconds (startTime));
        client->SetStopTime (Seconds (simDuration));
    }
    //fourth tcp client
    {
        Ptr<TcpClient>  client= CreateObject<TcpClient> (totalTxBytes);
        h1->AddApplication(client);
        client->ConfigurePeer(dummy_addr);
        client->SetCongestionAlgo(cong_algo);
        client->SetStartTime (Seconds (startTime));
        client->SetStopTime (Seconds (simDuration));
    }
    Simulator::Stop (Seconds (simDuration+10.0));
    Simulator::Run ();
    Simulator::Destroy ();
    return 0;
    
}
