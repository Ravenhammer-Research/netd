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
#include <memory>
#include <shared/include/base/serialization.hpp>

namespace netd {

enum class RequestType {
    GET,
    GET_CONFIG,
    EDIT_CONFIG,
    COMMIT,
    UNKNOWN
};

class Request : public base::Serialization<Request> {
public:
    Request() = default;
    explicit Request(const std::string& type, const std::string& data);
    Request(const std::string& type, const std::string& data, const std::string& messageId);
    virtual ~Request() = default;

    // Request properties
    virtual std::string getType() const { return type_; }
    virtual RequestType getRequestType() const;
    virtual std::string getData() const { return data_; }
    virtual std::string getMessageId() const { return messageId_; }
    void setType(const std::string& type) { type_ = type; }
    void setData(const std::string& data) { data_ = data; }
    void setMessageId(const std::string& messageId) { messageId_ = messageId; }

    // Generate unique message ID
    static std::string generateMessageId();

    // Parse request from XML string
    static std::unique_ptr<Request> fromString(const std::string& request);

    // Implement Serialization methods
    lyd_node* toYang(ly_ctx* ctx) const override;
    static Request fromYang(const ly_ctx* ctx, const lyd_node* node);

private:
    std::string type_;
    std::string data_;
    std::string messageId_;
};

// Concrete NETCONF request classes
class GetRequest : public Request {
public:
    GetRequest() = default;
    explicit GetRequest(const std::string& filter);
    virtual ~GetRequest() = default;

    std::string getType() const override { return "get"; }
    RequestType getRequestType() const override { return RequestType::GET; }
    std::string getData() const override { return filter_; }

    // Get-specific methods
    std::string getFilter() const { return filter_; }
    void setFilter(const std::string& filter) { filter_ = filter; }

    // Implement Serialization methods
    static GetRequest fromYang(const ly_ctx* ctx, const lyd_node* node);
private:
    std::string filter_;
};

class GetConfigRequest : public Request {
public:
    GetConfigRequest() = default;
    explicit GetConfigRequest(const std::string& source);
    GetConfigRequest(const std::string& source, const std::string& filter);
    virtual ~GetConfigRequest() = default;

    std::string getType() const override { return "get-config"; }
    RequestType getRequestType() const override { return RequestType::GET_CONFIG; }
    std::string getData() const override { return source_; }

    // GetConfig-specific methods
    std::string getSource() const { return source_; }
    std::string getFilter() const { return filter_; }
    void setSource(const std::string& source) { source_ = source; }
    void setFilter(const std::string& filter) { filter_ = filter; }

    // Implement Serialization methods
    static GetConfigRequest fromYang(const ly_ctx* ctx, const lyd_node* node);

private:
    std::string source_{"running"};
    std::string filter_;
};

class EditConfigRequest : public Request {
public:
    EditConfigRequest() = default;
    explicit EditConfigRequest(const std::string& target, const std::string& config);
    virtual ~EditConfigRequest() = default;

    std::string getType() const override { return "edit-config"; }
    RequestType getRequestType() const override { return RequestType::EDIT_CONFIG; }
    std::string getData() const override { return config_; }

    // EditConfig-specific methods
    std::string getTarget() const { return target_; }
    std::string getConfig() const { return config_; }
    void setTarget(const std::string& target) { target_ = target; }
    void setConfig(const std::string& config) { config_ = config; }

    // Implement Serialization methods
    static EditConfigRequest fromYang(const ly_ctx* ctx, const lyd_node* node);

private:
    std::string target_{"candidate"};
    std::string config_;
};

class CommitRequest : public Request {
public:
    CommitRequest() = default;
    virtual ~CommitRequest() = default;

    std::string getType() const override { return "commit"; }
    RequestType getRequestType() const override { return RequestType::COMMIT; }
    std::string getData() const override { return ""; }

    // Implement Serialization methods
    static CommitRequest fromYang(const ly_ctx* ctx, const lyd_node* node);
};

} // namespace netd

#endif // NETD_REQUEST_HPP
