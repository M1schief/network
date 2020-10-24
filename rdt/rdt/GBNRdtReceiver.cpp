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
//接收
void GBNRdtReceiver::receive(const Packet& packet)
{
	//检查校验和
	if (packet.checksum != pUtils->calculateCheckSum(packet))
	{
		pUtils->printPacket("接收方没有正确收到发送方的报文,数据校验错误", packet);
		pUtils->printPacket("接收方重新发送上次的确认报文", sndPkt);
		pns->sendToNetworkLayer(SENDER, sndPkt);
		return;
	}
	if (packet.seqnum != expectedSeqnum)
	{
		pUtils->printPacket("接收方没有正确收到发送方的报文,报文序号不对", packet);
		pUtils->printPacket("接收方重新发送上次的确认报文", sndPkt);
		pns->sendToNetworkLayer(SENDER, sndPkt);
		return;
	}
	pUtils->printPacket("接收方正确收到发送方的报文", packet);

	//取出Message，向上递交给应用层
	Message msg;
	memcpy(msg.data, packet.payload, sizeof(packet.payload));
	pns->delivertoAppLayer(RECEIVER, msg);

	sndPkt.acknum = packet.seqnum;
	sndPkt.checksum = pUtils->calculateCheckSum(sndPkt);
	pUtils->printPacket("接收方发送确认报文", sndPkt);
	pns->sendToNetworkLayer(SENDER, sndPkt);

	expectedSeqnum = (expectedSeqnum + 1) % seqSize;
}