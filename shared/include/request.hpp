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

#ifndef NETD_REQUEST_HPP
#define NETD_REQUEST_HPP

#include <string>
#include <shared/include/base/serialization.hpp>

namespace netd {

class Request : public base::Serialization<Request> {
public:
    Request() = default;
    explicit Request(const std::string& type, const std::string& data);
    virtual ~Request() = default;

    // Implement Serialization methods
    lyd_node* toYang() const override;
    static Request fromYang(const lyd_node* node);

    // Request properties
    virtual std::string getType() const { return type_; }
    virtual std::string getData() const { return data_; }
    void setType(const std::string& type) { type_ = type; }
    void setData(const std::string& data) { data_ = data; }

    // NETCONF-specific methods
    virtual std::string toNetconfRequest() const;
    static Request fromNetconfRequest(const std::string& request);

private:
    std::string type_;
    std::string data_;
};

// Concrete NETCONF request classes
class GetConfigRequest : public Request {
public:
    GetConfigRequest() = default;
    explicit GetConfigRequest(const std::string& source);
    virtual ~GetConfigRequest() = default;

    // Implement Request methods
    lyd_node* toYang() const override;
    static GetConfigRequest fromYang(const lyd_node* node);
    std::string getType() const override { return "get-config"; }
    std::string getData() const override { return source_; }
    std::string toNetconfRequest() const override;
    static GetConfigRequest fromNetconfRequest(const std::string& request);

    // GetConfig-specific methods
    std::string getSource() const { return source_; }
    void setSource(const std::string& source) { source_ = source; }

private:
    std::string source_{"running"};
};

class EditConfigRequest : public Request {
public:
    EditConfigRequest() = default;
    explicit EditConfigRequest(const std::string& target, const std::string& config);
    virtual ~EditConfigRequest() = default;

    // Implement Request methods
    lyd_node* toYang() const override;
    static EditConfigRequest fromYang(const lyd_node* node);
    std::string getType() const override { return "edit-config"; }
    std::string getData() const override { return config_; }
    std::string toNetconfRequest() const override;
    static EditConfigRequest fromNetconfRequest(const std::string& request);

    // EditConfig-specific methods
    std::string getTarget() const { return target_; }
    std::string getConfig() const { return config_; }
    void setTarget(const std::string& target) { target_ = target; }
    void setConfig(const std::string& config) { config_ = config; }

private:
    std::string target_{"candidate"};
    std::string config_;
};

class CommitRequest : public Request {
public:
    CommitRequest() = default;
    virtual ~CommitRequest() = default;

    // Implement Request methods
    lyd_node* toYang() const override;
    static CommitRequest fromYang(const lyd_node* node);
    std::string getType() const override { return "commit"; }
    std::string getData() const override { return ""; }
    std::string toNetconfRequest() const override;
    static CommitRequest fromNetconfRequest(const std::string& request);
};

} // namespace netd

#endif // NETD_REQUEST_HPP
