#pragma once
#include "RdtReceiver.h"

class SRRdtReceiver :public RdtReceiver
{
private:
	const int seqSize;
	const int winSize;
	int base;
	Packet* const sndPkt; //接收缓冲区
	bool* const sndState;	//缓冲区确认情况
	Packet lastPkt;

public:
	void receive(const Packet& packet);
	bool isInWin(int seqNum);

public:
	SRRdtReceiver();
	~SRRdtReceiver();
};