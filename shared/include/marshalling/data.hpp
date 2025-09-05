#ifndef NETD_MARSHALLING_DATA_HPP
#define NETD_MARSHALLING_DATA_HPP

#include <memory>
#include <shared/include/base/serialization.hpp>

namespace netd::shared::marshalling {

  // Base data container class
  class Data {
  public:
    Data();
    virtual ~Data() = default;

    // Store the lyd tree
    void setTree(lyd_node *tree) { tree_ = tree; }
    lyd_node *getTree() const { return tree_; }

    // Pure virtual methods
    virtual lyd_node *toYang(ly_ctx *ctx) const = 0;
    virtual std::unique_ptr<Data> fromYang(const ly_ctx *ctx,
                                           const lyd_node *node) = 0;
    virtual std::unique_ptr<Data> exportData() const = 0;

  private:
    lyd_node *tree_ = nullptr;
  };

} // namespace netd::shared::marshalling

#endif // NETD_MARSHALLING_DATA_HPP
