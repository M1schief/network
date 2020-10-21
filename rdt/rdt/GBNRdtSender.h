#pragma once
#include "RdtSender.h"

class GBNRdtSender :public RdtSender
{
private:
	int base;
	int nextSeqnum;
	const int winSize;	//�������ڴ�С
	const int seqSize;	//��Ŵ�С
	Packet* const sndPkt;

public:
	bool send(const Message& message);						//����Ӧ�ò�������Message����NetworkService����,������ͷ��ɹ��ؽ�Message���͵�����㣬����true;�����Ϊ���ͷ����ڵȴ�ȷ��״̬���ʹ����������ܾ�����Message���򷵻�false
	void receive(const Packet& ackPkt);						//����ȷ��Ack������NetworkService����	
	void timeoutHandler(int seqNum);					//Timeout handler������NetworkService����
	bool getWaitingState();								//����RdtSender�Ƿ��ڵȴ�״̬��������ͷ����ȴ�ȷ�ϻ��߷��ʹ�������������true

public:
	GBNRdtSender();
	virtual ~GBNRdtSender();
};