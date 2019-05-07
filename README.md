# ns3-congestion-variants
configure different congestion control algorithms  

put the file tcp-client under ns3-allinone-3.X/ns-3.X/src in ns3.  
put the tcp-main under scratch.  
And create a new fold names traces under ns3-allinone-3.X/ns-3.X/  
The reason is the following:  
```
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
```  
The mian logic to configure congestion control algorithm to a socket is the following:  
```
	ObjectFactory congestionAlgorithmFactory;
	congestionAlgorithmFactory.SetTypeId (id);
	Ptr<TcpCongestionOps> algo = congestionAlgorithmFactory.Create<TcpCongestionOps> ();
	TcpSocketBase *base=static_cast<TcpSocketBase*>(PeekPointer(m_socket));	
	base->SetCongestionControlAlgorithm (algo);
```

