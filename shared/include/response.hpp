/*
 * Copyright (c) 2024 Paige Thompson / Ravenhammer Research (paige@paige.bio)
 * 
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef NETD_RESPONSE_HPP
#define NETD_RESPONSE_HPP

#include <string>
#include <shared/include/base/serialization.hpp>

namespace netd {

class Response : public base::Serialization<Response> {
public:
    Response() = default;
    explicit Response(const std::string& messageId, bool success = true);
    Response(const std::string& messageId, const std::string& data, bool success = true);
    virtual ~Response() = default;

    // Implement Serialization methods
    lyd_node* toYang() const override;
    static Response fromYang(const lyd_node* node);

    // Response properties
    virtual std::string getMessageId() const { return messageId_; }
    virtual std::string getData() const { return data_; }
    virtual bool isSuccess() const { return success_; }
    
    void setMessageId(const std::string& messageId) { messageId_ = messageId; }
    void setData(const std::string& data) { data_ = data; }
    void setSuccess(bool success) { success_ = success; }

private:
    std::string messageId_;
    std::string data_;
    bool success_;
};

} // namespace netd

#endif // NETD_RESPONSE_HPP
