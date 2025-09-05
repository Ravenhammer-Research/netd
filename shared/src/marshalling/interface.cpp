/* (License header) */

#include <shared/include/exception.hpp>
#include <shared/include/marshalling/interface.hpp>

namespace netd::shared::marshalling {

  Interface::Interface() : Data() { data_ = nullptr; }

  Interface::Interface(lyd_node *node) : Data() { data_ = node; }

  void Interface::setData(lyd_node *node) { data_ = node; }

  lyd_node *Interface::getData() const { return data_; }

  lyd_node *Interface::toYang(ly_ctx *ctx) const {
    if (!ctx) {
      throw NotImplementedError(
          "Invalid YANG context provided to Interface::toYang");
    }
    // Simply return the stored data
    return data_;
  }

  std::unique_ptr<Data> Interface::fromYang([[maybe_unused]] const ly_ctx *ctx,
                                            const lyd_node *node) {
    if (!node) {
      throw NotImplementedError(
          "Invalid YANG node provided to Interface::fromYang");
    }

    auto interface = std::make_unique<Interface>();
    interface->setData(const_cast<lyd_node *>(node));
    return interface;
  }

  std::unique_ptr<Data> Interface::exportData() const {
    auto exported = std::make_unique<Interface>();
    exported->data_ = data_; // Copy the data pointer
    return exported;
  }

} // namespace netd::shared::marshalling
