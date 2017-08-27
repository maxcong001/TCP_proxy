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
#pragma once
#include "logger/logger.h"
#include "TCPProxy/util.h"
#include "translib/tcpServer.h"
#include "TCPProxy/messageBus.h"
#include "TCPProxy/TCPProxy.h"
using namespace translib;

namespace TCPProxy
{

class fakeServer : public translib::TcpServer
{
  public:
    static fakeServer *instance()
    {
        static fakeServer *ins = new fakeServer();
        return ins;
    }

  protected:
    virtual void onListenError()
    {
        __LOG(debug, "[fakeServer] listen fail");
    }

    virtual void onSessionRead(translib::TcpSession *session)
    {

        uint32_t length = session->getInputBufferLength();

        uint8_t buff[MAX_BUFF_LEN];

        session->readInputBuffer(buff, length);
        std::string str((char *)buff, length);

        __LOG(debug, "[fakeServer] receive message with length : " << length << " data is " << str);
        // send back message

        session->send((char *)buff, length);

        __LOG(debug, "[fakeServer] sesson id is : " << session->id());
    }

    virtual void onSessionDisconnected(translib::TcpSession *session)
    {
        char tmp_ip[sizeof(sockaddr_in)];
        session->getAddr((struct sockaddr_in *)tmp_ip, uint32_t(sizeof(sockaddr_in)));
        __LOG(debug, "[fakeServer] session with IP : " << std::string(tmp_ip, sizeof(sockaddr_in)) << " disconnected");
    }

    virtual void onNewSession(translib::TcpSession *session)
    {
        char tmp_ip[sizeof(sockaddr_in)];
        session->getAddr((struct sockaddr_in *)tmp_ip, uint32_t(sizeof(sockaddr_in)));
        __LOG(debug, "[fakeServer] new session with IP : " << std::string(tmp_ip, sizeof(sockaddr_in)));
    }
};
}