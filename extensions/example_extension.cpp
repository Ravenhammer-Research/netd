/*
 * Example NetD Extension - Native Backend
 *
 * This is a simple example extension that demonstrates how to create
 * a native backend extension for NetD.
 */

#include <iostream>
#include <netd/shared/extension.hpp>

class ExampleNativeBackend : public netd::shared::NetdExtension {
public:
  std::vector<netd::shared::ExtensionCapability>
  getCapabilities() const override {
    return {netd::shared::ExtensionCapability::NATIVE_BACKEND};
  }

  netd::shared::ExtensionInfo getInfo() const override {
    return {"example-native-backend",
            "1.0.0",
            "Example native backend extension for NetD",
            getCapabilities(),
            "NetD Team",
            "BSD-2-Clause"};
  }

  bool initialize() override {
    std::cout << "Example native backend extension initialized!" << std::endl;
    return true;
  }

  void cleanup() override {
    std::cout << "Example native backend extension cleaned up!" << std::endl;
  }

  bool isCompatible(const std::string &netd_version) const override {
    // Simple compatibility check - in real extensions, you might want
    // more sophisticated version checking
    return !netd_version.empty();
  }
};

// Required export function for dynamic loading
extern "C" netd::shared::NetdExtension *createExtension() {
  return new ExampleNativeBackend();
}
