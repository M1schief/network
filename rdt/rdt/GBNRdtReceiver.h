#pragma once
#include "RdtReceiver.h"

class GBNRdtReceiver :public RdtReceiver
{
private:
	int expectedSeqnum;
	const int seqSize;
	Packet sndPkt;

public:
	void receive(const Packet& packet);

public:
	GBNRdtReceiver();
	~GBNRdtReceiver();
};