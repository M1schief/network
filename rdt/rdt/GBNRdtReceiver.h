#pragma once
#include "RdtReceiver.h"

class GBNRdtReceiver :public RdtReceiver
{
private:
	const int seqSize; //��Ŵ�С
	int expectedSeqnum; //�ڴ����
	Packet sndPkt;

public:
	void receive(const Packet& packet);

public:
	GBNRdtReceiver();
	~GBNRdtReceiver();
};