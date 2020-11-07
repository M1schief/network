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
//缓存区是否满
bool TcpSender::getWaitingState()
{
	return (base + winSize) % seqSize == nextSeqnum % seqSize;
}
//序号是否正确
bool TcpSender::isInWin(int seqnum)
{
	if (base < (base + winSize) % seqSize)
		return seqnum >= base && seqnum < (base + winSize) % seqSize;
	else
		return seqnum >= base || seqnum < (base + winSize) % seqSize;
}
//发送
bool TcpSender::send(const Message& message)
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
//接收
void TcpSender::receive(const Packet& ackPkt)
{
	int checknum = pUtils->calculateCheckSum(ackPkt);
	if (checknum != ackPkt.checksum)
		pUtils->printPacket("接收ack损坏！", ackPkt);
	else
	{
		if (isInWin(ackPkt.acknum))
		{
			//正常ack
			cout << "接收ack" << ackPkt.acknum << endl;
			base = (ackPkt.acknum + 1) % seqSize;
			acknum = 1;
			pns->stopTimer(SENDER, 0);
			if (base != nextSeqnum)
				pns->startTimer(SENDER, Configuration::TIME_OUT, 0);
		}
		else
		{
			//冗余ack
			acknum++;
			pUtils->printPacket("收到多余ack", sndPkt[base]);
			if (acknum == 3)
				//快速重传
				pUtils->printPacket("快速重传", sndPkt[base]);
				pns->sendToNetworkLayer(RECEIVER, sndPkt[base]);
		}
	}
}

void TcpSender::timeoutHandler(int seqNum)
{
	cout << "超时\n";
	pns->stopTimer(SENDER, 0);
	pns->startTimer(SENDER, Configuration::TIME_OUT, 0);
	pUtils->printPacket("重发报文：", sndPkt[base]);
	pns->sendToNetworkLayer(RECEIVER, sndPkt[base]);
	cout << "重发完成！\n";
}