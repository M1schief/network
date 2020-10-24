#pragma once
#include "RdtSender.h"

class SRRdtSender :public RdtSender
{
private:
	int base;
	int nextSeqnum;
	const int winSize;	//�������ڴ�С
	const int seqSize;	//��Ŵ�С
	Packet* const sndPkt;
	bool* const sndState;

public:
	bool isInWin(int seqNum);
	bool send(const Message& message);						//����Ӧ�ò�������Message����NetworkService����,������ͷ��ɹ��ؽ�Message���͵�����㣬����true;�����Ϊ���ͷ����ڵȴ�ȷ��״̬���ʹ����������ܾ�����Message���򷵻�false
	void receive(const Packet& ackPkt);						//����ȷ��Ack������NetworkService����	
	void timeoutHandler(int seqNum);					//Timeout handler������NetworkService����
	bool getWaitingState();								//����RdtSender�Ƿ��ڵȴ�״̬��������ͷ����ȴ�ȷ�ϻ��߷��ʹ�������������true

public:
	SRRdtSender();
	virtual ~SRRdtSender();
};