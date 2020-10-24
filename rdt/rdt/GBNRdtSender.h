#pragma once
#include "RdtSender.h"

class GBNRdtSender :public RdtSender
{
private:
	int base; //�����
	int nextSeqnum; //��һ�����
	const int winSize;	//�������ڴ�С
	const int seqSize;	//��Ŵ�С
	Packet* const sndPkt; //���ͻ�����

public:
	bool send(const Message& message);	
	void receive(const Packet& ackPkt);		
	void timeoutHandler(int seqNum);
	bool getWaitingState();	
	
public:
	GBNRdtSender();
	~GBNRdtSender();
};