#include "SRRdtReceiver.h"
#include "Global.h"
#include "stdafx.h"

SRRdtReceiver::SRRdtReceiver() :
	seqSize(8), winSize(4), sndPkt(new Packet[8]), sndState(new bool[8])
{
	base = 1;

	for (int i = 0; i < seqSize; i++)
	{
		sndState[i] = false;
	}

	lastPkt.acknum = 0;
	lastPkt.seqnum = 0;
	memset(lastPkt.payload, ' ', Configuration::PAYLOAD_SIZE);
	lastPkt.checksum = pUtils->calculateCheckSum(lastPkt);
}

SRRdtReceiver::~SRRdtReceiver()
{
	delete[] sndPkt;
	delete[] sndState;
}
//�ж�����Ƿ�����ȷ������
bool SRRdtReceiver::isInWin(int seqnum)
{
	if (base < (base + winSize) % seqSize)
		return seqnum >= base && seqnum < (base + winSize) % seqSize;
	else
		return seqnum >= base || seqnum < (base + winSize) % seqSize;
}
//����
void SRRdtReceiver::receive(const Packet& packet)
{
	//���У���
	if (packet.checksum != pUtils->calculateCheckSum(packet))
	{
		pUtils->printPacket("���շ�û����ȷ�յ����ͷ��ı���,����У�����", packet);
		return;
	}
	else
	{
		if (!isInWin(packet.seqnum))
		{
			//����ָ��������
			pUtils->printPacket("���շ�û����ȷ�յ����ͷ��ı���,������Ų���", packet);
			lastPkt.acknum = packet.seqnum;
			lastPkt.seqnum = -1;
			memset(lastPkt.payload, ' ', Configuration::PAYLOAD_SIZE);
			lastPkt.checksum = pUtils->calculateCheckSum(lastPkt);
			pns->sendToNetworkLayer(SENDER, lastPkt); //��������ack���·��ͷ�
			return;
		}
		pUtils->printPacket("���շ���ȷ�յ����ͷ��ı���", packet);
		lastPkt.acknum = packet.seqnum;
		lastPkt.checksum = pUtils->calculateCheckSum(lastPkt);
		pUtils->printPacket("���շ�����ȷ�ϱ���", lastPkt);
		pns->sendToNetworkLayer(SENDER, lastPkt);
		sndPkt[packet.seqnum] = packet;
		sndState[packet.seqnum] = true;
		while (sndState[base])
		{
			//ȡ��Message�����ϵݽ���Ӧ�ò�
			Message msg;
			memcpy(msg.data, sndPkt[base].payload, sizeof(sndPkt[base].payload));
			pns->delivertoAppLayer(RECEIVER, msg);
			sndState[base] = false;
			base = (base + 1) % seqSize;
		}
	}
}