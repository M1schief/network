#include <cmath>
#include "TcpSender.h"
#include "DataStructure.h"
#include "stdafx.h"
#include "Global.h"

TcpSender::TcpSender() :
	winSize(4), seqSize(8), sndPkt(new Packet[8])
{
	base = 1;
	nextSeqnum = 1;
	acknum = 1;
}

TcpSender::~TcpSender()
{
	delete[] sndPkt;
}
//�������Ƿ���
bool TcpSender::getWaitingState()
{
	return (base + winSize) % seqSize == nextSeqnum % seqSize;
}
//����Ƿ���ȷ
bool TcpSender::isInWin(int seqnum)
{
	if (base < (base + winSize) % seqSize)
		return seqnum >= base && seqnum < (base + winSize) % seqSize;
	else
		return seqnum >= base || seqnum < (base + winSize) % seqSize;
}
//����
bool TcpSender::send(const Message& message)
{
	if (getWaitingState())
	{
		cout << "���ʹ���������\n";
		return false;
	}

	sndPkt[nextSeqnum].acknum = -1;
	sndPkt[nextSeqnum].seqnum = nextSeqnum;
	sndPkt[nextSeqnum].checksum = 0;
	memcpy(sndPkt[nextSeqnum].payload, message.data, sizeof(message.data));
	sndPkt[nextSeqnum].checksum = pUtils->calculateCheckSum(sndPkt[nextSeqnum]);
	pUtils->printPacket("���ͷ����ͱ��ģ�", sndPkt[nextSeqnum]);
	if (base == nextSeqnum)
		pns->startTimer(SENDER, Configuration::TIME_OUT, 0);
	pns->sendToNetworkLayer(RECEIVER, sndPkt[nextSeqnum]);
	nextSeqnum = (nextSeqnum + 1) % seqSize;
	return true;
}
//����
void TcpSender::receive(const Packet& ackPkt)
{
	int checknum = pUtils->calculateCheckSum(ackPkt);
	if (checknum != ackPkt.checksum)
		pUtils->printPacket("����ack�𻵣�", ackPkt);
	else
	{
		if (isInWin(ackPkt.acknum))
		{
			//����ack
			cout << "����ack" << ackPkt.acknum << endl;
			base = (ackPkt.acknum + 1) % seqSize;
			acknum = 1;
			pns->stopTimer(SENDER, 0);
			if (base != nextSeqnum)
				pns->startTimer(SENDER, Configuration::TIME_OUT, 0);
		}
		else
		{
			//����ack
			acknum++;
			pUtils->printPacket("�յ�����ack", sndPkt[base]);
			if (acknum == 3)
				//�����ش�
				pUtils->printPacket("�����ش�", sndPkt[base]);
				pns->sendToNetworkLayer(RECEIVER, sndPkt[base]);
		}
	}
}

void TcpSender::timeoutHandler(int seqNum)
{
	cout << "��ʱ\n";
	pns->stopTimer(SENDER, 0);
	pns->startTimer(SENDER, Configuration::TIME_OUT, 0);
	pUtils->printPacket("�ط����ģ�", sndPkt[base]);
	pns->sendToNetworkLayer(RECEIVER, sndPkt[base]);
	cout << "�ط���ɣ�\n";
}