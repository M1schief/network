#include <cmath>
#include "GBNRdtSender.h"
#include "DataStructure.h"
#include "stdafx.h"
#include "Global.h"

GBNRdtSender::GBNRdtSender() :
	winSize(4), seqSize(8), sndPkt(new Packet[8])
{
	base = 1;
	nextSeqnum = 1;
}

GBNRdtSender::~GBNRdtSender()
{
	delete[] sndPkt;
}

bool GBNRdtSender::getWaitingState()
{
	return (base + winSize) % seqSize == nextSeqnum % seqSize;
}

bool GBNRdtSender::send(const Message& message)
{
	if (getWaitingState())
	{
		cout << "发送窗口已满！\n";
		return false;
	}

	sndPkt[nextSeqnum].acknum = -1;
	sndPkt[nextSeqnum].seqnum = nextSeqnum;
	sndPkt[nextSeqnum].checksum = 0;
	memcpy(sndPkt[nextSeqnum].payload, message.data, sizeof(message.data));
	sndPkt[nextSeqnum].checksum = pUtils->calculateCheckSum(sndPkt[nextSeqnum]);
	pUtils->printPacket("发送方发送报文：", sndPkt[nextSeqnum]);
	if (base == nextSeqnum)
		pns->startTimer(SENDER, Configuration::TIME_OUT, 0);
	pns->sendToNetworkLayer(RECEIVER, sndPkt[nextSeqnum]);
	nextSeqnum = (nextSeqnum + 1) % seqSize;
	return true;
}

void GBNRdtSender::receive(const Packet& ackPkt)
{
	int checknum = pUtils->calculateCheckSum(ackPkt);
	if (checknum != ackPkt.checksum)
		pUtils->printPacket("接收ack损坏！", ackPkt);
	else
	{
		base = (ackPkt.acknum + 1) % seqSize;
		if (base == nextSeqnum)
			pns->stopTimer(SENDER, 0);
		else
		{
			pns->stopTimer(SENDER, 0);
			pns->startTimer(SENDER, Configuration::TIME_OUT, 0);
		}
		cout << "窗口滑动\n";
	}
}

void GBNRdtSender::timeoutHandler(int seqNum)
{
	cout << "超时\n";
	pns->stopTimer(SENDER, 0);
	pns->startTimer(SENDER, Configuration::TIME_OUT, 0);
	for (int i = base; i != nextSeqnum; i = (i + 1) % seqSize)
	{
		pUtils->printPacket("重发报文：", sndPkt[i]);
		pns->sendToNetworkLayer(RECEIVER, sndPkt[i]);
	}
	cout << "重发完成！\n";
}