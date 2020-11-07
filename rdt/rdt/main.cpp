// main.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Global.h"
#include "RdtSender.h"
#include "RdtReceiver.h"
#include "StopWaitRdtSender.h"
#include "StopWaitRdtReceiver.h"
#include "GBNRdtSender.h"
#include "GBNRdtReceiver.h"
#include "SRRdtSender.h"
#include "SRRdtReceiver.h"
#include "TcpSender.h"
#include "TcpReceiver.h"


int main(int argc, char* argv[])
{
	GBNRdtSender* GBNnSend = new GBNRdtSender();
	GBNRdtReceiver* GBNRecv = new GBNRdtReceiver();
	SRRdtSender* SRSend = new SRRdtSender();
	SRRdtReceiver* SRRecv = new SRRdtReceiver();
	TcpSender* TcpSend = new TcpSender();
	TcpReceiver* TcpRecv = new TcpReceiver();

//	pns->setRunMode(0);  //VERBOS模式
	pns->setRunMode(1);  //安静模式

	//pns->init();
	//pns->setRtdSender(GBNnSend);
	//pns->setRtdReceiver(GBNRecv);
	//pns->setInputFile("..\\file\\input.txt");
	//pns->setOutputFile("..\\file\\GBNoutput.txt");
	//pns->start();

	pns->init();
	pns->setRtdSender(SRSend);
	pns->setRtdReceiver(SRRecv);
	pns->setInputFile("..\\file\\input.txt");
	pns->setOutputFile("..\\file\\SRoutput.txt");
	pns->start(); 

	//pns->init();
	//pns->setRtdSender(TcpSend);
	//pns->setRtdReceiver(TcpRecv);
	//pns->setInputFile("..\\file\\input.txt");
	//pns->setOutputFile("..\\file\\TCPoutput.txt");
	//pns->start();

	delete GBNnSend;
	delete GBNRecv;
	delete SRSend;
	delete SRRecv;
	delete TcpSend;
	delete TcpRecv;
	delete pUtils;									//指向唯一的工具类实例，只在main函数结束前delete
	delete pns;										//指向唯一的模拟网络环境类实例，只在main函数结束前delete
	
	return 0;
}

