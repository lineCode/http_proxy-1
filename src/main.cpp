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


#include <stdio.h>
#include <thread>
#include <list>
#include <iostream>

#include <signal.h>

#include "net.h"
#include "utility.h"
#include "var.h"
#include "http_parse.h"


using namespace philippica_net;
using namespace std;

#define local_port 11010

void transmit(ConnetctionBase* a, ConnetctionBase* b)
{
    int readLen;
    char request[1024];
    while(readLen = a->receive(request, 1024))
    {
        if(readLen < 0)
        {
            break;
        }
        if(readLen == 0)
        {
            continue;
        }
        b->send(request , readLen);
    }
}


int getHost(ConnetctionBase* guest, char *request, HttpParser& parser)
{
    int retLen = guest->receive(request, 10240);
    for(int i = 1; i <= retLen; i++)
    {
        parser.getChar(request[i - 1]);
    }
    return retLen;
}

void httpsProcess(ConnetctionBase* guest, ConnetctionBase& pageGetter)
{
    thread send(transmit, guest, &pageGetter);
    thread receive(transmit, &pageGetter, guest);
    send.join();
    receive.join();
}

void tranmitPayload(ConnetctionBase* guest, ConnetctionBase& pageGetter)
{
    int readLen;
    char rsvMsg[1024];
    int payloadLen = -1;
    readLen = pageGetter.receiveLine(rsvMsg, 1024);
    if(readLen == 0)
    {
        return;
    }
    char magic[20];
    int httpType;
    sscanf(rsvMsg, "%s %d", magic, &httpType);

    guest -> send(rsvMsg, readLen);


    while(readLen = pageGetter.receiveLine(rsvMsg, 1024))
    {

        char *contentLenPtr = strstr(rsvMsg, "Content-Length");
        if(contentLenPtr != NULL)
        {
            contentLenPtr += 15;
            sscanf(contentLenPtr, "%d", &payloadLen);
        }
        guest -> send(rsvMsg, readLen);
        if(strlen(rsvMsg) == 2)
        {
            break;
        }
    }

    if(httpType == 404 || httpType == 201 || httpType == 204 || httpType == 304)
    {
        return;
    }

    if(payloadLen != -1)
    {
        for(int i = 1; i <= payloadLen; i++)
        {
            readLen = pageGetter.receive(rsvMsg, 1);
            guest -> send(rsvMsg, readLen);
        }
    }
    else
    {
        do
        {
            readLen = pageGetter.receiveLine(rsvMsg, 1024);
            if(readLen == 0)
            {
                break;
            }
            guest -> send(rsvMsg, readLen);
            sscanf(rsvMsg, "%x", &payloadLen);

            for(int i = 1; i <= payloadLen + 2; i++)
            {
                readLen = pageGetter.receive(rsvMsg, 1);
                guest -> send(rsvMsg, readLen);
            }

        }while(payloadLen != 0);
    }
}


void taskEntry(ServerConnection* guest)
{
    int requestLen;
    int readLen;
    int payloadLen;
    char request[10240];
    char rsvMsg[1024];
    bool first = false;
    char domainName[MAX_URL_LEN];
    ClientConnection pageGetter;
    for(;;)
    {
        HttpParser parser;
        payloadLen = -1;
        readLen = 0;

        readLen = getHost(guest, request, parser);
        domainName[0] = 0;
        parser.getDomain(domainName);


        if(domainName[0] == 0 && readLen > 0)
        {
            continue;
        }
        if(readLen <= 0)
        {
            delete guest;
            return;
        }

        pageGetter.connectByDomainName(domainName, parser.port);
        if(parser.port == 80)pageGetter.send(request, readLen);



        if(parser.port != 80)
        {
            if(first == false)
            {
                first = true;

                char *ch = "HTTP/1.1 200 Connection established\r\n\r\n";
                readLen = strlen(ch);
                guest -> send(ch , readLen);

                httpsProcess(guest, pageGetter);
                continue;
            }
            else
            {

                httpsProcess(guest, pageGetter);
                delete guest;
                return;
            }
        }
        else
        {
            tranmitPayload(guest, pageGetter);
        }
        pageGetter.shutdown();
        continue;
        printf("domain : %s\n" , domainName);
        readLen = guest->receive(request, 10240);
        readLen = pageGetter.send(request, readLen);
        tranmitPayload(guest, pageGetter);

    }
}

int main()
{
    Server server(local_port);
    server.init();
    signal(SIGPIPE, SIG_IGN);
    for(;;)
    {
        ServerConnection *a = new ServerConnection;
        server.setConnection(a);
        server.start();
        thread task(taskEntry, a);
        //task.join();
        task.detach();
    }
    return 0;
}
