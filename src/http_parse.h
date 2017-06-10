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


#ifndef _HTTP_PARSE__
#define _HTTP_PARSE__

#include "var.h"

typedef enum
{
    STATUS_OTHER = 0,
    STATUS_HOST_URL,
    STATUS_HOST_URL_PORT,
    STATUS_CR,
    STATUS_LF,
    STATUS_SPACE,
    STATUS_HOST_H,
    STATUS_HOST_HO,
    STATUS_HOST_HOS,
    STATUS_HOST_HOST,
    STATUS_HOST_COLON,
}HTTP_STATUS;

typedef enum
{

}URL_STATUS;


class HttpParser
{
public:
    HttpParser():status(STATUS_OTHER),urlLen(0),port(80){memset(url, 0, sizeof(url));};
    ~HttpParser(){};
    void getChar(char c);
    void getDomain(char *_url);
    bool hasGottenHost();
    int port;
private:
    void nextStatus(char c);
    HTTP_STATUS status;
    char url[MAX_URL_LEN];
    int urlLen;

};


void HttpParser::getDomain(char *_url)
{
    url[urlLen] = '\0';
    memcpy(_url, url, urlLen + 1);
}



#define SET_STATUS(LAST_STATUS, CUR_STATUS) \
    do{ \
        if(lastStatus == LAST_STATUS) \
            status = CUR_STATUS; \
    }while(0)

void HttpParser::nextStatus(char c)
{
    HTTP_STATUS lastStatus = status;
    switch(c)
    {
    case '\r':
        status = STATUS_CR;
        break;
    case '\n':
        SET_STATUS(STATUS_CR, STATUS_LF);
        break;
    case 'H':
        SET_STATUS(STATUS_LF, STATUS_HOST_H);
        break;
    case 'o':
        SET_STATUS(STATUS_HOST_H, STATUS_HOST_HO);
        break;
    case 's':
        SET_STATUS(STATUS_HOST_HO, STATUS_HOST_HOS);
        break;
    case 't':
        SET_STATUS(STATUS_HOST_HOS, STATUS_HOST_HOST);
        break;
    case ':':
        SET_STATUS(STATUS_HOST_HOST, STATUS_HOST_COLON);
        break;
    case ' ':
        if(lastStatus == STATUS_HOST_URL)
        {
            status = STATUS_OTHER;
        }
        break;
    default:
        status = STATUS_OTHER;
    }
}



void HttpParser::getChar(char c)
{
    switch(status)
    {
    case STATUS_HOST_COLON:
        status = STATUS_HOST_URL;
        break;
    case STATUS_HOST_URL:
        if(c == CR || c == LF)
        {
            status = STATUS_OTHER;
        }
        break;
    case STATUS_HOST_URL_PORT:
        if(c == CR || c == LF)
        {
            status = STATUS_OTHER;
        }
        break;
    default:
        break;
    }

    if(status == STATUS_HOST_URL)
    {
        if(c == ':')
        {
            status = STATUS_HOST_URL_PORT;
            port = 0;
        }
        else
        {
            if(c != SP)
            url[urlLen++] = c;
            url[urlLen] = '\0';
        }
        if(url[urlLen-2]=='m' && url[urlLen-3]=='o' && url[urlLen-4]=='c' && url[urlLen-5]=='.' )
        {
            puts("stop");
        }
    }
    else if(status == STATUS_HOST_URL_PORT)
    {
        port = port * 10 + c - '0';
    }
    else
    {
        nextStatus(c);
    }

}




#endif

