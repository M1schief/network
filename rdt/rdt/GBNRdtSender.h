#pragma once
#include "RdtSender.h"

class GBNRdtSender :public RdtSender
{
private:
	int base; //基序号
	int nextSeqnum; //下一个序号
	const int winSize;	//滑动窗口大小
	const int seqSize;	//序号大小
	Packet* const sndPkt; //发送缓存区

public:
	bool send(const Message& message);	
	void receive(const Packet& ackPkt);		
	void timeoutHandler(int seqNum);
	bool getWaitingState();	
	
public:
	GBNRdtSender();
	~GBNRdtSender();
};