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
    static void SetTraceFolder(const char *path);
    void OpenCwndTraceFile(std::string filename);
    void OpenRttTraceFile(std::string filename);
    void OpenBandwidthTraceFile(std::string filename);
    void OnCwnd(uint32_t oldval, uint32_t newval);
    void OnRtt(Time oldval, Time newval);
    void OnBandwidth(uint32_t seq,uint32_t size,uint64_t bps);
private:
    std::fstream m_cwnd;
    std::fstream m_rtt;
    std::fstream  m_bw;
};    
}
