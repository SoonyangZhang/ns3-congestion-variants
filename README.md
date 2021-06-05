# ns3-congestion-variants
##  Preparation  
The tcp-client module depends on ns-3.33.  
1 put the file tcp-client under ns3-allinone-3.33/ns-3.33/src  
2 Change the compilor flag in ns-allinone-3.33/ns-3.33/waf-tools/cflag.py  
Original version (line 22):  
```  
    self.warnings_flags = [['-Wall'], ['-Werror'], ['-Wextra']]  
```  
Changed version:  
```
    self.warnings_flags = [['-Wall'], ['-Wno-unused-parameter'], ['-Wextra']]  
```
3 Rebuild ns3  
```  
cd ns3-allinone-3.33/ns-3.33/  
./waf configure  
./waf build  
```  
4 copy tcp-test.cc to ns3-allinone-3.33/ns-3.33/scratch    
## Run  
```
cd ns3-allinone-3.33/ns-3.33/  
./waf --run "scratch/tcp-test --cong linux-reno"
```
The detail to conigure a specfic congestion control algorithms can be got:  
```  
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
        id=TcpLinuxReno::GetTypeId();   
    }   
    ObjectFactory congestionAlgorithmFactory;   
    congestionAlgorithmFactory.SetTypeId (id);  
    Ptr<TcpCongestionOps> algo = congestionAlgorithmFactory.Create<TcpCongestionOps> ();   
    TcpSocketBase *base=static_cast<TcpSocketBase*>(PeekPointer(m_socket));  
    base->SetCongestionControlAlgorithm (algo);  
}
```  
## Plot figures  
After running Done, the collected trace can be found under ns3-allinone-3.33/ns-3.33/traces/  
These script under plot folder can be used to plot figures.   
1 the way to plot congestion window:  
copy cwnd-plot.sh to ns3-allinone-3.33/ns-3.33/traces/  
```
chmod  777  cwnd-plot.sh  
./cwnd-plot.sh   
```
## Results
congestion window:  
![avatar](https://github.com/SoonyangZhang/ns3-congestion-variants/blob/master/results/cwnd.png)  
Inflight packets:  
![avatar](https://github.com/SoonyangZhang/ns3-congestion-variants/blob/master/results/inflight.png)  
Rtt:  
![avatar](https://github.com/SoonyangZhang/ns3-congestion-variants/blob/master/results/rtt.png)  
rate (sendrate) got by sender:  
![avatar](https://github.com/SoonyangZhang/ns3-congestion-variants/blob/master/results/sendrate.png)  
rate (goodput) got by receiver:  
![avatar](https://github.com/SoonyangZhang/ns3-congestion-variants/blob/master/results/goodput.png)  




