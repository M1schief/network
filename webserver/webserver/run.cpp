#include "Server.h"

void main()
{
	Server srv;
	if (srv.winsock_start() == -1)
		return;
	if (srv.server_start() == -1) 
		return;
	if (srv.listen_start() == -1) 
		return;
	if (srv.loop() == -1) 
		return;
}