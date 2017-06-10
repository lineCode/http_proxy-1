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
        //printf("transmit : %d\n", readLen);
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
    puts("thread back");
}


int getHost(ConnetctionBase* guest, char *request, HttpParser& parser)
{
    int retLen = guest->receive(request, 10240);

    //printf("%s\n",request);
    if(retLen >= 10240)
    {
        puts("That's full!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    }
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
        puts("sure");
        if(readLen == 0)
        {
            return;
        }


        char magic[20];
        int httpType;
        sscanf(rsvMsg, "%s %d", magic, &httpType);
        printf("%s\n", rsvMsg);

        guest -> send(rsvMsg, readLen);

        if(httpType == 302)
        {
                puts("302");
        }

        while(readLen = pageGetter.receiveLine(rsvMsg, 1024))
        {
            //printf("%s", rsvMsg);
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
        printf("http type : %d\n",httpType);

        if(httpType == 404 || httpType == 201 || httpType == 204 || httpType == 304)
        {
              //          readLen = pageGetter.receive(rsvMsg, 10240);
              //  guest -> send(rsvMsg, readLen);
            puts("not 200");
            return;
        }

        if(payloadLen != -1)
        {
            for(int i = 1; i <= payloadLen; i++)
            {
                readLen = pageGetter.receive(rsvMsg, 1);
                guest -> send(rsvMsg, readLen);
            }
            puts("over");
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
            puts("done");
        }
}


void taskEntry(ConnetctionBase* guest)
{
    int requestLen;
    int readLen;
    int payloadLen = -1;
    char request[10240];
    char rsvMsg[1024];
    ClientConnection pageGetter;
    bool first = false;
    int cnt=0;
    char domainName[MAX_URL_LEN];
    for(;;cnt++)
    {
        HttpParser parser;
        puts("1");
        payloadLen = -1;
        readLen = 0;
        //if(first == false)
        {
            puts("<><><>>><><><><<>get host");
            readLen = getHost(guest, request, parser);
            puts("readLen");
            domainName[0] = 0;
            parser.getDomain(domainName);
            printf("domain name : %s \n", domainName);
            char *contentLenPtr = strstr(domainName, "comer");
            if(contentLenPtr != NULL)
            {
                puts("123");
            }

            if(domainName[0] == 0 && readLen > 0)
            {
                puts("what the fuck");
                continue;
            }
            if(readLen <= 0)
            {
                puts("delete guest 209");
                printf("id = %d\n",guest->id);
                delete guest;
                return;
            }
            puts("first");
            if(domainName[0] == 'p')
            {
                puts("passport");
            }
            pageGetter.connectByDomainName(domainName, parser.port);
            if(parser.port == 80)pageGetter.send(request, readLen);
            //printf("request : %s", request);



            // CONNECT PACKAGE
            if(parser.port != 80)
            {
                if(first == false)
                {
                    first = true;
                    puts("domain");
                    char *ch = "HTTP/1.1 200 Connection established\r\n\r\n";
                    readLen = strlen(ch);
                    guest -> send(ch , readLen);

                    httpsProcess(guest, pageGetter);
                puts("delete guest 237");
                printf("id = %d\n",guest->id);
                    delete guest;
                    continue;
                }
                else
                {
                    puts("33");
                    httpsProcess(guest, pageGetter);
                puts("delete guest 237");
                printf("id = %d\n",guest->id);
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
        }
        printf("hahaha : \n%d %s\n",readLen, request);

        //readLen = pageGetter.receive(request, 10240);
        printf("<><>>><><<<><>>cnt == %d\n", cnt);
        printf("domain : %s\n" , domainName);

        if(domainName[0] == 'k' && cnt == 1)
        {
            puts("kb");
        }

        readLen = guest->receive(request, 10240);
        printf("no not me\n", cnt);
        readLen = pageGetter.send(request, readLen);
        // Get state
        puts("suspect");

        tranmitPayload(guest, pageGetter);

    }


}

int main()
{
    Server server;
    server.setLocalPort(local_port);
    server.build();
    signal(SIGPIPE, SIG_IGN);
    int id = 0;
    for(;;id++)
    {
        ConnetctionBase* guest = server.wait();
        guest->id = id;
        thread task(taskEntry, guest);
        //task.join();
        task.detach();
    }
    return 0;
}
