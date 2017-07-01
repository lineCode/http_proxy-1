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


#ifndef _NET_H__
#define _NET_H__

#include <cstring>
#include <unistd.h>


#include "var.h"
#include "utility.h"



namespace philippica_net
{
    class ConnetctionBase;
    class Server;
    class ClientConnection;
    class Guest;


	class ConnetctionBase
	{
	public:
		ConnetctionBase():fileDescription(UN_INIT_VAR)
		{
		};

        virtual ~ConnetctionBase()
		{
			shutdown();
		};

		// Shutdown the connetion.
		void shutdown();

		int receive(char *text, const int bufLen);

		// Include the last two character: "\r\n"
		int receiveLine(char *text, const int bufLen);

		int send(const char *text, const int bufLen);

		void setFileDescription(int _fileDescription)
		{
            fileDescription = _fileDescription;
		}
		int id;

	protected:
		unsigned int fileDescription;
	};




	class ClientConnection: public ConnetctionBase
	{
	public:
		ClientConnection():remotePort(DEFAULT_HTTP_PORT)
			{

			};

		~ClientConnection()
		{
		};
		inline void setRemotePort(int _port)
		{
			remotePort = _port;
			int2Str(remotePort, strRemotePort);
		}
		void connectByDomainName(const char *domain, int port = 80);
		void connectByIP(const char *IpAddr);

	private:
		char *ipAddr[MAX_IP_LENGTH + 1];
		int remotePort;
		char strRemotePort[MAX_PORT_LENGTH + 1];
	};



	class ServerConnection: public ConnetctionBase
	{
	public:
		ServerConnection()
        {
        };

		virtual ~ServerConnection()
		{
            shutdown();
		};

		virtual void run(){};

		Guest* wait();

	private:
		char *ipAddr[MAX_IP_LENGTH + 1];
		int localPort;
		char strLocalPort[MAX_PORT_LENGTH + 1];
	};




	class Server
	{
	public:
        Server(){};
		Server(int port)
		{
            setLocalPort(port);
        }
		Server(ServerConnection* server, int port = 80)
		{
            setLocalPort(port);
        }
        void setConnection(ServerConnection* server);
		~Server(){};
		inline void setLocalPort(int _port)
		{
			localPort = _port;
			int2Str(localPort, strLocalPort);
		}
		void start();
		void init();

    private:
		int localPort;
		char strLocalPort[MAX_PORT_LENGTH + 1];
		ServerConnection* server;
		unsigned int fileDescription;
	};


} // End namespace philippica_net



#endif

