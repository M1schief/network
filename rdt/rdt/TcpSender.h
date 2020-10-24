#pragma once
#include "RdtSender.h"

class TcpSender :public RdtSender
{
private:
	int base;
	int nextSeqnum;
	int acknum;
	const int winSize;	//滑动窗口大小
	const int seqSize;	//序号大小
	Packet* const sndPkt;

public:
	bool isInWin(int seqNum);
	bool send(const Message& message);
	void receive(const Packet& packet);
	void timeoutHandler(int seqNum);
	bool getWaitingState();

public:
	TcpSender();
	~TcpSender();
};