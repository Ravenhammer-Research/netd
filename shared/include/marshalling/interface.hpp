#ifndef NETD_MARSHALLING_INTERFACE_HPP
#define NETD_MARSHALLING_INTERFACE_HPP

#include <memory>
#include <shared/include/marshalling/data.hpp>
#include <vector>

namespace netd::shared::marshalling {

  // Interface class that stores serialized interface data
  class Interface : public Data {
  public:
    Interface();
    Interface(lyd_node *node);
    virtual ~Interface() = default;

    lyd_node *toYang(ly_ctx *ctx) const override;
    std::unique_ptr<Data> fromYang(const ly_ctx *ctx,
                                   const lyd_node *node) override;
    std::unique_ptr<Data> exportData() const override;

    // Set the serialized interface data
    void setData(lyd_node *node);

    // Get the serialized interface data
    lyd_node *getData() const;

  private:
    lyd_node *data_ = nullptr;
  };

} // namespace netd::shared::marshalling

#endif // NETD_MARSHALLING_INTERFACE_HPP
