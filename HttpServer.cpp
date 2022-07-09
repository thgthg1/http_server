#include <iostream>
#include <string>
#include "HttpServer.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

using namespace std;

HttpServer::HttpServer()
{
}

HttpServer::~HttpServer()
{
	if (listen_fd > 0)
	{
		close(listen_fd);
		listen_fd = -1;
	}
}

bool HttpServer::init_server(unsigned short port)
{
	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == listen_fd)
	{
		cerr << "Error: failed to create socket" << endl;
		return false;
	}
	
	bool opt = false;
	setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(&opt));
	
	sockaddr_in  saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);
	saddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if (bind(listen_fd, (sockaddr*)&saddr, sizeof(saddr)) != 0)
	{
		cerr << "Error: failed to bind port (" << port << ")!" << endl;
		return false;
	}
	
	if (listen(listen_fd, 5) < 0)
	{
		cerr << "Error: failed to listen!!!" << endl;
		return false;
	}
	
	epfd = epoll_create(FD_SIZE);
	add_event(listen_fd, EPOLLIN);
	
	return true;
}
void HttpServer::start_serving()
{
	for (;;)
	{
		int ret = epoll_wait(epfd, events, EPOLLEVENTS_SIZE, -1);
		for (int i = 0; i < ret; i++)
		{
			if (events[i].data.fd == listen_fd)
			{
				sockaddr_in caddr;
				socklen_t len = sizeof(caddr);
				int client = accept(listen_fd, (sockaddr*)&caddr, &len);
				if (client <= 0)
				{
					cerr << "==> Accept client failed!!!" << endl;
					continue;
				}
				add_event(client, EPOLLIN);
			}
			else
			{
				int client = events[i].data.fd;
				if (events[i].events & EPOLLIN)
				{
					bool re = false;
					
					if (!re)
					{
						char buf[BUFFER_SIZE] = { 0 };
						int revclen = recv(client, buf, BUFFER_SIZE, 0);
						if (revclen <= 0)
						{
							close(client);
							delete_event(client);
							client_data[client].request = "";
							continue;
						}
						client_data[client].request += buf;
						re = BUFFER_SIZE != revclen;
						if (!re)
							continue;
					}
					if (!client_data[client].parse_request())
					{
						close(client);
						delete_event(client);
						client_data[client].request = "";
						continue;
					}
					
					modify_event(client, EPOLLOUT);
				}
				else if (events[i].events & EPOLLOUT)
				{
					char buf[BUFFER_SIZE] = { 0 };
					int data_read = client_data[client].read_data(buf, BUFFER_SIZE);
					int data_sent = send(client, buf, data_read, MSG_NOSIGNAL);
					if (data_sent == 0)
						continue;
					if (data_sent < 0)
					{
						delete_event(client);
						client_data[client].close();
						close(client);
						continue;
					}
					client_data[client].forward(data_sent);
					
					if (client_data[client].is_reading_finished())
					{
						delete_event(client);
						client_data[client].close();
						close(client);
					}
				}
			}
		}
	}
	close(listen_fd);
}
void HttpServer::add_event(int fd, int state)
{
	struct epoll_event ev;
	ev.data.fd = fd;
	ev.events = state;
	epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
}

void HttpServer::modify_event(int fd, int state)
{
	struct epoll_event ev;
	ev.data.fd = fd;
	ev.events = state;
	epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev);
}

void HttpServer::delete_event(int fd)
{
	epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
}


