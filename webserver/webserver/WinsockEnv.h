#pragma once

class WinsockEnv
{
private:
	WinsockEnv(void);
	~WinsockEnv();
public:
	static int Startup();
};
