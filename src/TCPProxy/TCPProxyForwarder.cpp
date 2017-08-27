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
#include "TCPProxy/TCPProxyForwarder.h"
#include "TCPProxy/TCPProxy.h"
namespace TCPProxy
{

TCPProxyForwarder::TCPProxyForwarder(forwarder_conn_info info)
{
    _info = info;
    auto bus = message_bus<TCPProxyForwarder>::instance();

    bus->register_handler(DELETE_F, (void *)this, [&](void *objp, void *msgp, std::string topic) {
        if (!objp)
        {
            return;
        }
        if (!msgp)
        {
            return;
        }

        msg_c2s msg = unpack_msg_c2s((char *)msgp);

        forwarder_conn_info tmp;

        tmp.str_to_forwarder_conn_info(std::string((char *)(msg.msg), msg.len));

        this->stop_f((forwarder_conn_info *)(&msgp));
    });

    // start listener
    ss.start_TCPProxyserver(info);
}

void TCPProxyForwarder::start()
{
    ss.wait();
}
void TCPProxyForwarder::stop()
{
    ss.stop();
}
void TCPProxyForwarder::stop_f(forwarder_conn_info *msgp)
{
    // check if the message is ourself
    if (*msgp == _info)
    {
        // do the stop work
        stop();

        // send to message bus
        auto bus = message_bus<TCPProxyForwarder>::instance();
        bus->call(DELETE_F_INS, this);
    }
    else
    {
        // not us
        return;
    }
}
}