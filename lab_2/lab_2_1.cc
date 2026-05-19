#include <print>
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <tuple>
#include <algorithm>

namespace lab_2
{
  class Part
  {
   public:
    Part(std::string_view const name, std::uint32_t const id)
      : m_name(name)
      , m_id(id)
    {}

    [[nodiscard]] std::string_view name() const { return this->m_name; }

    [[nodiscard]] std::uint32_t id() const { return this->m_id; }

   private:
    std::string m_name;
    std::uint32_t m_id;
  };

  class Warehouse
  {
   public:
    [[nodiscard]] static Warehouse& instance()
    {
      static auto instance = Warehouse();
      return instance;
    }

    Warehouse(Warehouse const&) = delete;
    Warehouse(Warehouse&&) = delete;
    Warehouse& operator=(Warehouse const&) = delete;
    Warehouse& operator=(Warehouse&&) = delete;

    [[nodiscard]] std::size_t count() const { return this->m_inventory.size(); }

    Warehouse& add_part(std::string_view const name, std::uint32_t const id)
    {
      [[maybe_unused]] auto const _lock = std::scoped_lock(this->m_mtx);

      this->m_inventory.push_back(std::make_unique<Part>(name, id));
      std::println("Added: {} (ID: {})", name, id);
      return *this;
    }

    void list_parts() const
    {
      [[maybe_unused]] auto const _lock = std::scoped_lock(this->m_mtx);

      std::println("Inventory (Total: {}) ---", this->m_inventory.size());
      for(auto const& part : this->m_inventory)
        std::println(" - [{}] Name: {}", part->id(), part->name());
    }

    Warehouse& operator+=(std::tuple<std::string_view, std::uint32_t> const& data)
    {
      auto const& [name, id] = data;
      return this->add_part(name, id);
    }

   private:
    Warehouse() = default;

    mutable std::mutex m_mtx;
    std::vector<std::unique_ptr<Part>> m_inventory;
  };
}  // namespace lab_2

int main()
{
  using namespace lab_2;

  Warehouse::instance()
    .add_part("square part", 1'024)
    .add_part("round part", 2'048)
    .add_part("cylinder part", 1'337);

  Warehouse::instance() += {"complex part", 4'096};
  {
    auto factory_worker = std::jthread([]() {
      Warehouse::instance() += {"fan", 69};
      Warehouse::instance() += {"heater", 420};
      Warehouse::instance() += {"hair dryer", 42};
    });
  }

  Warehouse::instance().list_parts();

  return 0;
}
