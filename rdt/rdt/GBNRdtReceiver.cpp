#include "GBNRdtReceiver.h"
#include "Global.h"
#include "stdafx.h"

GBNRdtReceiver::GBNRdtReceiver() :
	seqSize(8)
{
	expectedSeqnum = 1;
	sndPkt.acknum = 0;
	sndPkt.seqnum = 0;
	memset(sndPkt.payload, ' ', Configuration::PAYLOAD_SIZE);
	sndPkt.checksum = pUtils->calculateCheckSum(sndPkt);
}

GBNRdtReceiver::~GBNRdtReceiver()
{
}
//����
void GBNRdtReceiver::receive(const Packet& packet)
{
	//���У���
	if (packet.checksum != pUtils->calculateCheckSum(packet))
	{
		pUtils->printPacket("���շ�û����ȷ�յ����ͷ��ı���,����У�����", packet);
		pUtils->printPacket("���շ����·����ϴε�ȷ�ϱ���", sndPkt);
		pns->sendToNetworkLayer(SENDER, sndPkt);
		return;
	}
	if (packet.seqnum != expectedSeqnum)
	{
		pUtils->printPacket("���շ�û����ȷ�յ����ͷ��ı���,������Ų���", packet);
		pUtils->printPacket("���շ����·����ϴε�ȷ�ϱ���", sndPkt);
		pns->sendToNetworkLayer(SENDER, sndPkt);
		return;
	}
	pUtils->printPacket("���շ���ȷ�յ����ͷ��ı���", packet);

	//ȡ��Message�����ϵݽ���Ӧ�ò�
	Message msg;
	memcpy(msg.data, packet.payload, sizeof(packet.payload));
	pns->delivertoAppLayer(RECEIVER, msg);

	sndPkt.acknum = packet.seqnum;
	sndPkt.checksum = pUtils->calculateCheckSum(sndPkt);
	pUtils->printPacket("���շ�����ȷ�ϱ���", sndPkt);
	pns->sendToNetworkLayer(SENDER, sndPkt);

	expectedSeqnum = (expectedSeqnum + 1) % seqSize;
}