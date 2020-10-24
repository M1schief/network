#pragma once
#include "RdtReceiver.h"

class SRRdtReceiver :public RdtReceiver
{
private:
	const int seqSize;
	const int winSize;
	int base;
	Packet* const sndPkt;
	bool* const sndState;
	Packet lastPkt;

public:
	void receive(const Packet& packet);
	bool isInWin(int seqNum);

public:
	SRRdtReceiver();
	~SRRdtReceiver();
};