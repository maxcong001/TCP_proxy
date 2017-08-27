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
#include <string>
#include <map>
#include <functional>
#include <iostream>
#include <utility>
#include <mutex>
#include <tuple>
namespace TCPProxy
{
template <typename OBJ>
class message_bus
{
  public:
    static message_bus<OBJ> *instance()
    {
        static auto ins = new message_bus<OBJ>();
        return ins;
    }

    typedef std::function<void(void *, void *, std::string topic)> handler_f;
    void register_handler(std::string const &name, void *t, handler_f f)
    {
        std::lock_guard<std::mutex> lck(mtx);
#if 0
        for (auto it = invokers_.begin(); it != invokers_.end(); it++)
        {
            if (std::get<1>(it->second) == t)
            {
                invokers_.erase(it);
            }
        }
#endif
        (this->invokers_).emplace(name, std::make_tuple(f, t));
    }

    void call(const std::string &name, void *message)
    {
        std::lock_guard<std::mutex> lck(mtx);
        auto it = invokers_.equal_range(name);

        for (auto iter = it.first; iter != it.second; iter++)
        {
            handler_f cb;
            void *obj;
            std::tie(cb, obj) = iter->second;
            cb((void *)obj, message, name);
        }
    }

    void remove_handler(std::string const &name, void *t)
    {
        std::lock_guard<std::mutex> lck(mtx);
        for (auto it = invokers_.begin(); it != invokers_.end(); it++)
        {
            if (std::get<1>(it->second) == t)
            {
                invokers_.erase(it);
            }
        }
    }

  private:
    std::multimap<std::string, std::tuple<handler_f, void *>> invokers_;
    std::mutex mtx;
};
#if 0
int main()
{
    int a = 0;
#if 0
    message_bus<int> bus;
  

    bus.register_handler("test", &a, [](void *objp, void *msgp, std::string topic) { std::cout << "objp is : " << (void *)objp << " msgp is : " << (void *)msgp << " topic is : " << topic << std::endl; });
    bus.register_handler("test", &a, [](void *objp, void *msgp, std::string topic) { std::cout << "objp isxxxx : " << (void *)objp << " msgp is : " << (void *)msgp << " topic is : " << topic << std::endl; });
    bus.call("test", (void *)0);

    bus.remove_handler("test", &a);
    bus.call("test", (void *)0);
#endif
    auto bus = message_bus<int>::instance();
    bus->register_handler("test", &a, [](void *objp, void *msgp, std::string topic) { std::cout << "objp is : " << (void *)objp << " msgp is : " << (void *)msgp << " topic is : " << topic << std::endl; });
    bus->register_handler("test", &a, [](void *objp, void *msgp, std::string topic) { std::cout << "objp isxxxx : " << (void *)objp << " msgp is : " << (void *)msgp << " topic is : " << topic << std::endl; });
    bus->call("test", (void *)0);

    bus->remove_handler("test", &a);
    bus->call("test", (void *)0);
}
#endif
}