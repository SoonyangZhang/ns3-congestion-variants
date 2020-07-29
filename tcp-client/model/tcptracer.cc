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
void TcpTracer::OnCwnd(uint32_t oldval, uint32_t newval)
{
    if(m_cwnd.is_open()){
        char line [255];
        oldval=oldval;
        sprintf (line, "%16f %16f",
			Simulator::Now().GetSeconds(),(double)newval/1460.0);
        m_cwnd << line << std::endl;        
    }

}
void TcpTracer::OnRtt(Time oldval, Time newval)
{
    if(m_rtt.is_open()){
        char line [255];
        oldval=oldval;
        sprintf (line, "%16f %16d",
			Simulator::Now().GetSeconds(),(uint32_t)newval.GetMilliSeconds ());
        m_rtt<< line << std::endl;        
    }
	
}
}
