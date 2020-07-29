#include "tcptracer.h"
#include <unistd.h>
#include <memory.h>
namespace ns3{
TcpTracer::~TcpTracer()
{
    CloseRttTraceFile();
    CloseTcpWindowTraceFile();
}
void TcpTracer::OpenCwndTraceFile(std::string filename)
{

	char buf[FILENAME_MAX];
	std::string path = std::string (getcwd(buf, FILENAME_MAX)) 
    + "/traces/" + filename+"_cwnd.txt";;
	m_cwnd.open(path.c_str(), std::fstream::out);
}
void TcpTracer::CloseTcpWindowTraceFile()
{
	if (m_cwnd.is_open())
		m_cwnd.close();
}
void TcpTracer::OpenRttTraceFile(std::string filename)
{
    char buf[FILENAME_MAX];
    std::string path = std::string (getcwd(buf, FILENAME_MAX)) 
    + "/traces/" + filename+"_rtt.txt";
    m_rtt.open(path.c_str(), std::fstream::out);
}
void TcpTracer::CloseRttTraceFile()
{
	if (m_rtt.is_open())
        m_rtt.close();
}
void TcpTracer::OpenBandwidthTraceFile(std::string filename){
    char buf[FILENAME_MAX];
    std::string path = std::string (getcwd(buf, FILENAME_MAX)) 
    + "/traces/" + filename+"_bw.txt";
    m_bw.open(path.c_str(), std::fstream::out);    
}
void TcpTracer::CloseBandwidhtTraceFile(){
	if (m_bw.is_open())
        m_bw.close();    
}
void TcpTracer::OnCwnd(uint32_t oldval, uint32_t newval)
{
    if(m_cwnd.is_open()){
        oldval=oldval;
        float now=Simulator::Now().GetSeconds();
        uint32_t cwnd=newval/1460;
        m_cwnd <<now<<"\t"<<cwnd<< std::endl;        
    }

}
void TcpTracer::OnRtt(Time oldval, Time newval)
{
    if(m_rtt.is_open()){
        oldval=oldval;
        float now=Simulator::Now().GetSeconds();
        m_rtt<<now<<"\t"<<(uint32_t)newval.GetMilliSeconds()<< std::endl;        
    }
	
}
void TcpTracer::OnBandwidth(uint32_t seq,uint32_t size,uint64_t bps){
    if(m_bw.is_open()){
        float now=Simulator::Now().GetSeconds();
        float rate=bps/1000;
        m_bw<<now<<"\t"<<seq<<"\t"<<size<<"\t"<<rate<<std::endl;
    }
}
}
