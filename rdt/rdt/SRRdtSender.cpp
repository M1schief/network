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
//是否在正确窗口内
bool SRRdtSender::isInWin(int seqnum)
{
	if (base < (base + winSize) % seqSize)
		return seqnum >= base && seqnum < (base + winSize) % seqSize;
	else
		return seqnum >= base || seqnum < (base + winSize) % seqSize;
}
//缓存区是否满
bool SRRdtSender::getWaitingState()
{
	return (base + winSize) % seqSize == nextSeqnum % seqSize;
}
//发送
bool SRRdtSender::send(const Message& message)
{
	//判断缓存区空间
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
	pns->startTimer(SENDER, Configuration::TIME_OUT, nextSeqnum);
	pns->sendToNetworkLayer(RECEIVER, sndPkt[nextSeqnum]);
	nextSeqnum = (nextSeqnum + 1) % seqSize;
	return true;
}

void SRRdtSender::receive(const Packet& ackPkt)
{
	int checknum = pUtils->calculateCheckSum(ackPkt);
	if (checknum != ackPkt.checksum)
		pUtils->printPacket("接收ack损坏！", ackPkt);
	else
	{
		pns->stopTimer(SENDER, ackPkt.acknum);
		if (isInWin(ackPkt.acknum))
		{
			sndState[ackPkt.acknum] = true;
			while (sndState[base])
			{
				//窗口更新
				sndState[base] = false;
				base = (base + 1) % seqSize;
			}
			cout << "窗口滑动\n";
		}
	}
}

void SRRdtSender::timeoutHandler(int seqNum)
{
	cout << "超时\n";

	pns->stopTimer(SENDER, seqNum);
	pUtils->printPacket("重发报文：", sndPkt[seqNum]);
	pns->sendToNetworkLayer(RECEIVER, sndPkt[seqNum]);
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);

	cout << "重发完成！\n";
}