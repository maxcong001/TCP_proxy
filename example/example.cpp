/*
 * Copyright (c) 2016-20017 Max Cong <savagecm@qq.com>
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <iostream>
#include <string>
#include <map>
#include "TCPProxy/TCPProxyCommandServer.h"
#include "TCPProxy/TCPProxyClient.h"
#include "TCPProxy/fakeServer.h"
#include "TCPProxy/appClient.h"
#include "TCPProxy/util.h"
#include "translib/define.h"

using namespace std;

void start_command_server()
{
	auto ins = TCPProxy::TCPProxyCommandServer::instance();
	ins->wait();
}
void app_send_message(TCPProxy::AppClient &ac)
{
	__LOG(debug, "now send message");
	if (ac.send("aaa", 2))
	{
		__LOG(debug, "app client send message success");
	}
	else
	{
		__LOG(error, "app client send message fail");
	}
	sleep(1);
	if (ac.send("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", 10))
	{
		__LOG(debug, "app client send message success");
	}
	else
	{
		__LOG(error, "app client send message fail");
	}
}

int main(int argc, char *argv[])
{
	//port:
	//command server  2567
	//TCPProxy server 2568
	//fake redis 2565

	// now start fake redis
	__LOG(debug, "now start fake redis");
	TCPProxy::fakeServer fr;
	if (!fr.listen("127.0.0.1", 2565))
	{
		__LOG(error, " fake redis listen return fail!");
	}

	// redis TCPProxyulator thread
	__LOG(debug, "now start redis TCPProxyulator");
	TCPProxy::TCPProxy rs;
	auto rs_f = std::bind(&TCPProxy::TCPProxy::init, &rs);
	std::thread TCPProxy_thread(rs_f);
	TCPProxy_thread.detach();
	sleep(1);

	//TCPProxyulator client thread
	__LOG(debug, "now start client");
	Loop loop;
	TCPProxy::SimClient sc(loop);
	if (!sc.connect("127.0.0.1", CS_PORT))
	{
		__LOG(error, " connector connect to command server return fail!");
	}
	loop.start(true);
	sleep(1);

	//  just for test. start a forwarder
	TCPProxy::forwarder_conn_info tmp = TCPProxy::forwarder_conn_info("127.0.0.1", "2568", "127.0.0.1", "2565");
	std::string tmp_str = tmp.seralize();
	std::string message = TCPProxy::pack_msg_c2s(TCPProxy::ADD_FORWARDER, tmp_str.c_str(), (uint32_t)tmp_str.size());
	__LOG(debug, "message after pack is " << message);
	sc.send(message.c_str(), (uint32_t)(message.size()));
	sleep(1);

	// now start the APP client
	Loop loop1;
	__LOG(debug, "now start the APP client");
	TCPProxy::AppClient ac(loop1);
	if (!ac.connect("127.0.0.1", 2568))
	{
		__LOG(error, " app client connect to TCPProxy server return fail!");
	}
	loop1.start();

	// now App send message
	sleep(1);
	app_send_message(ac);
	sleep(1);
	app_send_message(ac);

	// now update the delay timer
	uint32_t delay = 10000;
	std::string message_001 = TCPProxy::pack_msg_c2s(TCPProxy::UPDATE_DELAY, (char *)(&delay), (uint32_t)(sizeof(uint32_t)));
	__LOG(debug, "update the delay timer to 10s");
	sc.send(message_001.c_str(), (uint32_t)(message_001.size()));

	// now App send message
	sleep(1);
	app_send_message(ac);
	sleep(1);
	app_send_message(ac);

	loop.wait();
	__LOG(error, "exit example!");
	return 0;
}
