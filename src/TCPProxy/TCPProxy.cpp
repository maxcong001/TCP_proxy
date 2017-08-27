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
#include "TCPProxy/TCPProxy.h"
#include "TCPProxy/TCPProxyCommandServer.h"
#include "TCPProxy/TCPProxyForwarder.h"
#include <functional>
namespace TCPProxy
{
//using namespace std::placeholders;
bool TCPProxy::init()
{
    __LOG(debug, "[TCPProxy] init is called!");
    auto cs = TCPProxyCommandServer::instance();
    if (!cs->listen("127.0.0.1", CS_PORT))
    {
        __LOG(error, "command server listen failed, check the port");
        return false;
    }
    __LOG(debug, "[TCPProxy] command server listen success on port : " << CS_PORT);
    //auto _func = std::bind(&TCPProxy::add, this);
    auto bus = message_bus<TCPProxyForwarder>::instance();
    bus->register_handler(ADD_F, this, [&](void *objp, void *msgp, std::string topic) {
        if (!objp)
        {
            return;
        }
        if (!msgp)
        {
            return;
        }
        // new a forwarder and record the pointer in the internal map
        // start a new thread
        msg_c2s msg = unpack_msg_c2s((char *)msgp);

        forwarder_conn_info tmp;

        tmp.str_to_forwarder_conn_info(std::string((char *)(msg.msg), msg.len));
        tmp.dump();
        using namespace std::placeholders;
        auto _func = std::bind(&TCPProxy::add, this, tmp);
        std::thread forwarder_t(_func);
        forwarder_t.detach();
        //add((forwarder_conn_info *)msgp);
    });

    bus->register_handler(DELETE_F_INS, this, [&](void *objp, void *msgp, std::string topic) {
        if (!objp)
        {
            return;
        }
        if (!msgp)
        {
            return;
        }
        // delete form internal map
        ((TCPProxy *)objp)->delete_f((TCPProxyForwarder *)msgp);
    });
    __LOG(debug, "[TCPProxy] register handler success, then call redis command server wait function wait for client's input");
    cs->wait();
    return true;
}

void TCPProxy::add(forwarder_conn_info &info)
{
    std::lock_guard<std::mutex> lck(mtx);
    __LOG(debug, "[TCPProxy] now new a TCPProxy forwarder, IP info :");
    info.dump();

    auto tmp = new TCPProxyForwarder(info);
    __LOG(debug, "[TCPProxy] add the TCPProxyForwarder to the forwarder map");
    f_map.emplace(tmp);
    tmp->start();
    __LOG(error, "[TCPProxy] exit the new forwarder thread");
}
bool TCPProxy::delete_f(TCPProxyForwarder *msgp)
{
    std::lock_guard<std::mutex> lck(mtx);
    __LOG(debug, "[TCPProxy] now delete a TCPProxy forwarder");
    msgp->stop();
    f_map.erase(msgp);
    delete msgp;
    msgp = NULL;
    return true;
}
}