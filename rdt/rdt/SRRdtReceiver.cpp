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
//判断序号是否在正确窗口内
bool SRRdtReceiver::isInWin(int seqnum)
{
	if (base < (base + winSize) % seqSize)
		return seqnum >= base && seqnum < (base + winSize) % seqSize;
	else
		return seqnum >= base || seqnum < (base + winSize) % seqSize;
}
//接收
void SRRdtReceiver::receive(const Packet& packet)
{
	//检查校验和
	if (packet.checksum != pUtils->calculateCheckSum(packet))
	{
		pUtils->printPacket("接收方没有正确收到发送方的报文,数据校验错误", packet);
		return;
	}
	else
	{
		if (!isInWin(packet.seqnum))
		{
			//不在指定窗口内
			pUtils->printPacket("接收方没有正确收到发送方的报文,报文序号不对", packet);
			lastPkt.acknum = packet.seqnum;
			lastPkt.seqnum = -1;
			memset(lastPkt.payload, ' ', Configuration::PAYLOAD_SIZE);
			lastPkt.checksum = pUtils->calculateCheckSum(lastPkt);
			pns->sendToNetworkLayer(SENDER, lastPkt); //正常发送ack更新发送方
			return;
		}
		pUtils->printPacket("接收方正确收到发送方的报文", packet);
		lastPkt.acknum = packet.seqnum;
		lastPkt.checksum = pUtils->calculateCheckSum(lastPkt);
		pUtils->printPacket("接收方发送确认报文", lastPkt);
		pns->sendToNetworkLayer(SENDER, lastPkt);
		sndPkt[packet.seqnum] = packet;
		sndState[packet.seqnum] = true;
		while (sndState[base])
		{
			//取出Message，向上递交给应用层
			Message msg;
			memcpy(msg.data, sndPkt[base].payload, sizeof(sndPkt[base].payload));
			pns->delivertoAppLayer(RECEIVER, msg);
			sndState[base] = false;
			base = (base + 1) % seqSize;
		}
	}
}