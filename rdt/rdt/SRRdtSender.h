#pragma once
#include "RdtSender.h"

class SRRdtSender :public RdtSender
{
private:
	int base;
	int nextSeqnum;
	const int winSize;	//滑动窗口大小
	const int seqSize;	//序号大小
	Packet* const sndPkt; //发送缓存
	bool* const sndState; //缓存确认

public:
	bool isInWin(int seqNum);
	bool send(const Message& message);	
	void receive(const Packet& ackPkt);	
	void timeoutHandler(int seqNum);
	bool getWaitingState();	

public:
	SRRdtSender();
	virtual ~SRRdtSender();
};