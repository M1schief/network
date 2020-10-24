#pragma once
#include "RdtReceiver.h"

class GBNRdtReceiver :public RdtReceiver
{
private:
	const int seqSize; //ÐòºÅ´óÐ¡
	int expectedSeqnum; //ÆÚ´ýÐòºÅ
	Packet sndPkt;

public:
	void receive(const Packet& packet);

public:
	GBNRdtReceiver();
	~GBNRdtReceiver();
};