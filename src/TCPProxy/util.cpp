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
#include "TCPProxy/util.h"
namespace TCPProxy
{
msg_c2s unpack_msg_c2s(char *buff)
{
    msg_c2s ret;
    type_c2s *tmp = (type_c2s *)buff;
    ret.type = *tmp;
    tmp++;
    uint32_t *tmp_len = (uint32_t *)tmp;
    ret.len = *tmp_len;
    tmp_len++;
    ret.msg = (void *)tmp_len;
    __LOG(debug, " unpack message: type is " << ret.type << " length is " << ret.len << " message is " << std::string((char *)(ret.msg), ret.len));

    return ret;
}
std::string pack_msg_c2s(type_c2s type, const char *msg, uint32_t len)
{
    type_c2s tmp = type;
    uint32_t tmp_len = len;
    return std::string((char *)(&tmp), sizeof(type_c2s)) + std::string((char *)(&tmp_len), sizeof(uint32_t)) + std::string(msg, len);
}
}