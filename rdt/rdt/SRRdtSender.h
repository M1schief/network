#pragma once
#include "RdtSender.h"

class SRRdtSender :public RdtSender
{
private:
	int base;
	int nextSeqnum;
	const int winSize;	//�������ڴ�С
	const int seqSize;	//��Ŵ�С
	Packet* const sndPkt; //���ͻ���
	bool* const sndState; //����ȷ��

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