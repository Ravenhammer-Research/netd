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

#include <client/include/processor/command.hpp>
#include <sstream>

namespace netd::client::processor {

  Command::Command()
      : command_type_(CommandType::SET_CMD), unit_number_(0), vlan_id_(0),
        vlan_tagging_(false), display_mode_(DisplayMode::NONE) {}

  void Command::setCommandType(CommandType type) { command_type_ = type; }

  CommandType Command::getCommandType() const { return command_type_; }

  void Command::setInterfaceName(const std::string &name) {
    interface_name_ = name;
  }

  const std::string &Command::getInterfaceName() const {
    return interface_name_;
  }

  void Command::setUnitNumber(int unit) { unit_number_ = unit; }

  int Command::getUnitNumber() const { return unit_number_; }

  void Command::setIpAddress(const std::string &ip) { ip_address_ = ip; }

  const std::string &Command::getIpAddress() const { return ip_address_; }

  void Command::setDescription(const std::string &desc) { description_ = desc; }

  const std::string &Command::getDescription() const { return description_; }

  void Command::setVlanId(int vlan) { vlan_id_ = vlan; }

  int Command::getVlanId() const { return vlan_id_; }

  void Command::setSpeedValue(const std::string &speed) {
    speed_value_ = speed;
  }

  const std::string &Command::getSpeedValue() const { return speed_value_; }

  void Command::setVlanTagging(bool tagging) { vlan_tagging_ = tagging; }

  bool Command::getVlanTagging() const { return vlan_tagging_; }

  void Command::setIdentifier(const std::string &id) { identifier_ = id; }

  const std::string &Command::getIdentifier() const { return identifier_; }

  void Command::setStringValue(const std::string &str) { string_value_ = str; }

  const std::string &Command::getStringValue() const { return string_value_; }

  void Command::setDisplayMode(DisplayMode mode) { display_mode_ = mode; }

  DisplayMode Command::getDisplayMode() const { return display_mode_; }

  void Command::reset() {
    command_type_ = CommandType::SET_CMD;
    interface_name_.clear();
    unit_number_ = 0;
    ip_address_.clear();
    description_.clear();
    vlan_id_ = 0;
    speed_value_.clear();
    identifier_.clear();
    string_value_.clear();
    vlan_tagging_ = false;
    display_mode_ = DisplayMode::NONE;
  }

  bool Command::isValid() const {
    // Basic validation - at least one field should be set
    return !interface_name_.empty() || !ip_address_.empty() ||
           !description_.empty() || !identifier_.empty() ||
           !string_value_.empty() || unit_number_ > 0 || vlan_id_ > 0;
  }

  std::string Command::toString() const {
    std::ostringstream oss;

    switch (command_type_) {
    case CommandType::SET_CMD:
      oss << "SET ";
      break;
    case CommandType::DELETE_CMD:
      oss << "DELETE ";
      break;
    case CommandType::SHOW_CMD:
      oss << "SHOW ";
      break;
    case CommandType::COMMIT_CMD:
      oss << "COMMIT";
      return oss.str();
    case CommandType::EDIT_CMD:
      oss << "EDIT ";
      break;
    case CommandType::QUIT_CMD:
      oss << "QUIT";
      return oss.str();
    case CommandType::HELP_CMD:
      oss << "HELP";
      return oss.str();
    }

    if (!interface_name_.empty()) {
      oss << "INTERFACES " << interface_name_;
      if (unit_number_ > 0) {
        oss << " UNIT " << unit_number_;
      }
      if (!ip_address_.empty()) {
        oss << " FAMILY INET ADDRESS " << ip_address_;
      }
      if (!description_.empty()) {
        oss << " DESCRIPTION \"" << description_ << "\"";
      }
      if (vlan_id_ > 0) {
        oss << " VLAN_ID " << vlan_id_;
      }
      if (!speed_value_.empty()) {
        oss << " SPEED " << speed_value_;
      }
      if (vlan_tagging_) {
        oss << " VLAN_TAGGING";
      }
    }

    if (!identifier_.empty()) {
      oss << " ROUTING_INSTANCES " << identifier_;
    }

    if (!string_value_.empty()) {
      oss << " \"" << string_value_ << "\"";
    }

    switch (display_mode_) {
    case DisplayMode::BRIEF_MODE:
      oss << " BRIEF";
      break;
    case DisplayMode::DETAIL_MODE:
      oss << " DETAIL";
      break;
    case DisplayMode::EXTENSIVE_MODE:
      oss << " EXTENSIVE";
      break;
    case DisplayMode::TERSE_MODE:
      oss << " TERSE";
      break;
    case DisplayMode::NONE:
      break;
    }

    return oss.str();
  }

} // namespace netd::client::processor
