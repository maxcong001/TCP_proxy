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
#include "TCPProxy/TCPProxyConnector.h"
#include "translib/tcpServer.h"
#include <unordered_map>

using namespace translib;
namespace TCPProxy
{

class TCPProxyServer : public translib::TcpServer
{
  public:
    typedef std::shared_ptr<TCPProxyConnector> sc_ptr_p;
    typedef std::shared_ptr<Loop> loop_ptr_p;
    static TCPProxyServer *instance()
    {
        static TCPProxyServer *ins = new TCPProxyServer();
        return ins;
    }
    bool start_TCPProxyserver(forwarder_conn_info info)
    {
        _info = info;
        if (listen((info.serverIP).c_str(), uint16_t(std::stoi(info.serverPort))))
        {
            __LOG(debug, "[TCPProxyServer] listen success, IP is : " << (info.serverIP).c_str() << " port is : " << uint16_t(std::stoi(info.serverPort)));
            return true;
        }
        else
        {
            __LOG(error, "[TCPProxyServer] listen fail, IP is : " << (info.serverIP).c_str() << " port is : " << uint16_t(std::stoi(info.serverPort)));
            return false;
        }
    }

  protected:
    virtual void onListenError()
    {
        __LOG(debug, " TCPProxy sercer listen return error");
    }

    virtual void onSessionRead(translib::TcpSession *session)
    {

        uint32_t length = session->getInputBufferLength();

        uint8_t buff[MAX_BUFF_LEN];

        session->readInputBuffer(buff, length);
        std::string str((char *)buff, length);

        __LOG(debug, "receive message with length : " << length << " data is " << str << " now sizeof session map is : " << session_map.size());

        for (auto it : session_map)
        {
            if (it.first == session)
            {
                if ((it.second)->send((char *)buff, length))
                {
                    __LOG(debug, "[TCPProxyServer] now send message via connector");
                    break;
                }
                else
                {
                    __LOG(error, "[TCPProxyServer] connector send message to redis return fail");
                    return;
                }
            }
        }

        __LOG(debug, "[TCPProxyServer] sesson id is : " << session->id());
    }

    virtual void onSessionDisconnected(translib::TcpSession *session)
    {
        session_map.erase(session_map.find(session));
        loop_map.erase(loop_map.find(session));
        __LOG(debug, " run here");
    }

    virtual void onNewSession(translib::TcpSession *session)
    {

        __LOG(debug, "[TCPProxyServer] new session, now connect to redis, IP is : " << _info.connectorIP << ":" << _info.connectorPort);
        // new a connector and bind to a session
        loop_ptr_p loop(new Loop());
        sc_ptr_p sc(new TCPProxyConnector(*loop));

        if (!(sc->connect_with_IP(_info.connectorIP, _info.connectorPort)))
        {
            __LOG(error, " connector connect to IP : " << _info.connectorIP << " fail");
            return;
        }
        loop->start(true);

        sc->setSession(session);
        session_map.emplace(session, sc);
        loop_map.emplace(session, loop);
        __LOG(debug, " run here");
    }

  public:
  private:
    forwarder_conn_info _info;
    std::unordered_map<translib::TcpSession *, sc_ptr_p> session_map;
    std::unordered_map<translib::TcpSession *, loop_ptr_p> loop_map;
};
}