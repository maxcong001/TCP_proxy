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
#include "util/singleton.h"
#include "TCPProxy/util.h"
#include "translib/tcpClient.h"
#include "translib/tcpSession.h"
#include "translib/timerManager.h"
using namespace translib;
namespace TCPProxy
{
class TCPProxyConnector : public translib::TcpClient
{
  public:
    TCPProxyConnector(const translib::Loop &loop) : translib::TcpClient(loop)
    {
        _connected = false;
        _delay = 0;
        init();
    }
    bool init();

    bool connect_with_IP(std::string IP, std::string Port)
    {
        __LOG(debug, "[TCPProxyConnector] connect to " << IP << ":" << Port);
        _IP = IP;
        _Port = Port;
        return connect(IP.c_str(), uint16_t(std::stoi(Port)));
    }
    bool isConnected()
    {
        return _connected;
    }
    void setDelay(uint32_t delay)
    {
        delay = _delay;
    }

  protected:
    virtual void onRead()
    {
        uint32_t length = getInputBufferLength();
        // tmp solution, later use malloc according to the message length
        msg_with_len *tmp = new msg_with_len();
        tmp->length = length;

        readInputBuffer((uint8_t *)(tmp->msg), length);
        __LOG(debug, "[TCPProxyConnector]  get buffer " << std::string(((char *)tmp->msg), length) << " length is : " << length);
        if (!_delay)
        {
            (this->session)->send((char *)(((msg_with_len *)tmp)->msg), ((msg_with_len *)tmp)->length);
            __LOG(error, "[TCPProxyConnector]  send buffer to client with len : " << ((msg_with_len *)tmp)->length);
            //(this->session)->send("aa", length);
            delete (msg_with_len *)tmp;
            tmp = NULL;
        }
        else
        {
            auto timerM = Singleton<translib::TimerManager>::Instance();
            auto timer = timerM->getTimer();
            timer->startOnce(_delay, (void *)tmp, [&](void *usrdata) {

                (this->session)->send((char *)(((msg_with_len *)usrdata)->msg), ((msg_with_len *)usrdata)->length);
                __LOG(error, "[TCPProxyConnector]  send buffer to client with len : " << ((msg_with_len *)usrdata)->length);
                //(this->session)->send("aa", length);
                delete (msg_with_len *)usrdata;
                usrdata = NULL;

            });
        }
    }

    virtual void onDisconnected()
    {
        reconnect();
        __LOG(debug, " run here");
    }

    virtual void onConnected(int error)
    {
        __LOG(debug, " run here");
    }
    virtual bool reconnect()
    {
        return connect_with_IP(_IP, _Port);
    }

  public:
    virtual void setSession(translib::TcpSession *sessionP)
    {
        session = sessionP;
    }

  private:
    std::string _IP;
    std::string _Port;
    translib::TcpSession *session;
    bool _connected;
    uint32_t _delay;
};
}