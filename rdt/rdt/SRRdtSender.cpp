#include "SRRdtSender.h"
#include "DataStructure.h"
#include "stdafx.h"
#include "Global.h"

SRRdtSender::SRRdtSender() :
	winSize(4), seqSize(8), sndPkt(new Packet[8]), sndState(new bool[8])
{
	for (int i = 0; i < seqSize; i++)
	{
		sndState[i] = false;
	}
	base = 1;
	nextSeqnum = 1;
}

SRRdtSender::~SRRdtSender()
{
	delete[] sndPkt;
	delete[] sndState;
}
//�Ƿ�����ȷ������
bool SRRdtSender::isInWin(int seqnum)
{
	if (base < (base + winSize) % seqSize)
		return seqnum >= base && seqnum < (base + winSize) % seqSize;
	else
		return seqnum >= base || seqnum < (base + winSize) % seqSize;
}
//�������Ƿ���
bool SRRdtSender::getWaitingState()
{
	return (base + winSize) % seqSize == nextSeqnum % seqSize;
}
//����
bool SRRdtSender::send(const Message& message)
{
	//�жϻ������ռ�
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
	pns->startTimer(SENDER, Configuration::TIME_OUT, nextSeqnum);
	pns->sendToNetworkLayer(RECEIVER, sndPkt[nextSeqnum]);
	nextSeqnum = (nextSeqnum + 1) % seqSize;
	return true;
}

void SRRdtSender::receive(const Packet& ackPkt)
{
	int checknum = pUtils->calculateCheckSum(ackPkt);
	if (checknum != ackPkt.checksum)
		pUtils->printPacket("����ack�𻵣�", ackPkt);
	else
	{
		pns->stopTimer(SENDER, ackPkt.acknum);
		if (isInWin(ackPkt.acknum))
		{
			sndState[ackPkt.acknum] = true;
			while (sndState[base])
			{
				//���ڸ���
				sndState[base] = false;
				base = (base + 1) % seqSize;
			}
			cout << "���ڻ���\n";
		}
	}
}

void SRRdtSender::timeoutHandler(int seqNum)
{
	cout << "��ʱ\n";

	pns->stopTimer(SENDER, seqNum);
	pUtils->printPacket("�ط����ģ�", sndPkt[seqNum]);
	pns->sendToNetworkLayer(RECEIVER, sndPkt[seqNum]);
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);

	cout << "�ط���ɣ�\n";
}