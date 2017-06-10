/*
 * Copyright (C) 2017 Yuanjie Pu(philippica)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */


#include <netdb.h>















#include "net.h"



using namespace philippica_net;


void ClientConnection::connectByDomainName(const char *domain, int port)
{

    setRemotePort(port);
	unsigned int &clientfd = fileDescription;
	struct addrinfo hints, *listp, *p;
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_ADDRCONFIG | AI_NUMERICSERV;
	getaddrinfo(domain, strRemotePort, &hints, &listp);
	for(p = listp; p; p = p -> ai_next)
	{
		char ipbuf[20];
		clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		getnameinfo(p->ai_addr, p->ai_addrlen, ipbuf, 100, NULL, 0, NI_NUMERICHOST);
		if(clientfd < 0)
		{
		    continue;
		}
		if(connect(clientfd, p->ai_addr, p->ai_addrlen) != -1)
		{
		    break;
		}
		close(clientfd);
	}
	freeaddrinfo(listp);
}


void ConnetctionBase::shutdown()
{
    if(fileDescription != UN_INIT_VAR)
    {
        close(fileDescription);
    }
    fileDescription = UN_INIT_VAR;
}

int ConnetctionBase::receive(char *text, const int bufLen)
{
    int length;
    length = read(fileDescription, (void*)text, bufLen);
    return length;
}

int ConnetctionBase::send(const char *text, const int bufLen)
{
    int length;
    length = write(fileDescription, (void*)text, bufLen);
    return length;
}

int ConnetctionBase::receiveLine(char *text, const int bufLen)
{
    int i, retValOfRead;
    for(i = 0; i < bufLen - 1; i++)
    {
        retValOfRead = read(fileDescription, text + i, 1);
        if(retValOfRead != 1)
        {
            break;
        }
        if(*(text + i) == '\n')
        {
            *(text + i + 1) = '\0';
            break;
        }
    }
    if(i == 0)
    {
        return retValOfRead;
    }
    if(i == bufLen - 2)
    {
        *(text + bufLen - 1) = '\0';
    }
    return i + 1;
}




void Server::build()
{

    unsigned int &listenfd = fileDescription;
    int optval = 1;
    struct addrinfo hints, *listp, *p;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG | AI_NUMERICSERV;
    getaddrinfo(NULL, strLocalPort, &hints, &listp);

    for(p = listp; p; p = p -> ai_next)
    {
        listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if(listenfd < 0)
        {
            continue;
        }
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));
        if(bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
        {
            break;
        }
        close(listenfd);
    }
    freeaddrinfo(listp);
    if(listen(listenfd, LISTENQ) < 0)
    {
        close(listenfd);
    }
}


Guest* Server::wait()
{
    struct sockaddr_storage clientAddr;
    socklen_t clientlen = sizeof(sockaddr_storage);
    int connfd = accept(fileDescription, (sockaddr*)&clientAddr, &clientlen);
    Guest* ret = new Guest;
    ret->setFileDescription(connfd);
    return ret;
}










