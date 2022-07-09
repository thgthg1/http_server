#include "HttpServer.h"

#include <stdlib.h>
#include <string.h>
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
	// 端口处理
	unsigned short port = 80;
	if (argc > 3)
	{
		cerr << "At most two arguments." << endl;
		return -1;
	}
	if (argc == 2)
		port = atoi(argv[1]);
	else if (argc == 3)
	{
		if (strcmp(argv[1], "--port") != 0)
		{
			cerr << "Unknown command \"" << argv[1] << "\"" << endl;
			return -2;
		}
		port = atoi(argv[2]);
	}
	
	// 新建服务器
	HttpServer *server = new HttpServer();
	
	if (!server->init_server(port))
	{
		cerr << "Failed in initializing server!" << endl;
		return -3;
	}
	
	// 启动服务器
	server->start_serving();
	delete server;
	return 0;
}

