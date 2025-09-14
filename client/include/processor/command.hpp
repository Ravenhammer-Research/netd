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

#ifndef NETD_CLIENT_PROCESSOR_COMMAND_HPP
#define NETD_CLIENT_PROCESSOR_COMMAND_HPP

#include <string>

namespace netd::client::processor {

enum class CommandType {
    SET_CMD,
    DELETE_CMD,
    SHOW_CMD,
    COMMIT_CMD,
    EDIT_CMD,
    QUIT_CMD
};

enum class DisplayMode {
    NONE,
    BRIEF_MODE,
    DETAIL_MODE,
    EXTENSIVE_MODE,
    TERSE_MODE
};

class Command {
public:
    Command();
    ~Command() = default;

    // Command type
    void setCommandType(CommandType type);
    CommandType getCommandType() const;

    // Interface configuration
    void setInterfaceName(const std::string& name);
    const std::string& getInterfaceName() const;

    void setUnitNumber(int unit);
    int getUnitNumber() const;

    void setIpAddress(const std::string& ip);
    const std::string& getIpAddress() const;

    void setDescription(const std::string& desc);
    const std::string& getDescription() const;

    void setVlanId(int vlan);
    int getVlanId() const;

    void setSpeedValue(const std::string& speed);
    const std::string& getSpeedValue() const;

    void setVlanTagging(bool tagging);
    bool getVlanTagging() const;

    // Routing configuration
    void setIdentifier(const std::string& id);
    const std::string& getIdentifier() const;

    void setStringValue(const std::string& str);
    const std::string& getStringValue() const;

    // Display options
    void setDisplayMode(DisplayMode mode);
    DisplayMode getDisplayMode() const;

    // Utility methods
    void reset();
    bool isValid() const;
    std::string toString() const;

private:
    CommandType command_type_;
    std::string interface_name_;
    int unit_number_;
    std::string ip_address_;
    std::string description_;
    int vlan_id_;
    std::string speed_value_;
    std::string identifier_;
    std::string string_value_;
    bool vlan_tagging_;
    DisplayMode display_mode_;
};

} // namespace netd::client::processor

#endif // NETD_CLIENT_PROCESSOR_COMMAND_HPP
