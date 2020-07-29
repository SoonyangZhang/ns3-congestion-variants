#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include "ns3/nstime.h"
#include "ns3/simulator.h"
namespace ns3{
class TcpTracer
{
public:
	TcpTracer(){}
    ~TcpTracer();
    void OpenCwndTraceFile(std::string filename);
    void CloseTcpWindowTraceFile();
    void OpenRttTraceFile(std::string filename);
    void CloseRttTraceFile();
    void OnCwnd(uint32_t oldval, uint32_t newval);
    void OnRtt(Time oldval, Time newval);
private:
    std::fstream m_cwnd;
    std::fstream m_rtt;
};    
}
