#include <print>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <expected>
#include <string_view>
#include <stdexcept>

#include "../money.h"

namespace lab_3
{
  class IPricingStrategy
  {
   public:
    virtual ~IPricingStrategy() = default;

    [[nodiscard]] virtual Money calculate(Money base_price) const = 0;
  };

  class StandardPricing final : public IPricingStrategy
  {
   public:
    [[nodiscard]] Money calculate(Money const base_price) const override { return base_price; }
  };

  class FixedDiscountPricing final : public IPricingStrategy
  {
   public:
    explicit FixedDiscountPricing(Money const discount)
      : m_discount(discount)
    {}

    [[nodiscard]] Money calculate(Money const base) const override
    {
      return base - this->m_discount;
    }

   private:
    Money m_discount;
  };

  class Tariff
  {
    std::string m_destination;
    Money m_base_price;
    std::unique_ptr<IPricingStrategy> m_strategy;

   public:
    Tariff(
      std::string_view const dest,
      Money const base,
      std::unique_ptr<IPricingStrategy>&& strategy
    )
      : m_destination(dest)
      , m_base_price(base)
      , m_strategy(std::move(strategy))
    {
      if(this->m_destination.empty())
        throw std::runtime_error("Empty destination");
    }

    [[nodiscard]] Money effective_price() const
    {
      return this->m_strategy->calculate(this->m_base_price);
    }

    [[nodiscard]] std::string_view destination() const { return this->m_destination; }
  };

  enum class AirportError
  {
    EmptyInventory,
    InternalFailure
  };

  class Airport
  {
    std::vector<Tariff> m_tariffs;

   public:
    void add_tariff(Tariff&& tariff) { this->m_tariffs.push_back(std::move(tariff)); }

    [[nodiscard]] std::expected<Tariff const*, AirportError> find_max_cost_tariff() const noexcept
    {
      if(this->m_tariffs.empty())
        return std::unexpected(AirportError::EmptyInventory);
      auto it = std::ranges::max_element(this->m_tariffs, std::less {}, &Tariff::effective_price);
      return &(*it);
    }
  };
}  // namespace lab_3

int main()
{
  using namespace lab_3;
  auto airport = Airport();

  try {
    airport.add_tariff(
      Tariff("Moscow", Money::from_decimal(45000.0), std::make_unique<StandardPricing>())
    );
    airport.add_tariff(Tariff(
      "SPB",
      Money::from_decimal(30000.0),
      std::make_unique<FixedDiscountPricing>(Money::from_decimal(5000.0))
    ));
    airport.add_tariff(
      Tariff("Belgrade", Money::from_decimal(85000.0), std::make_unique<StandardPricing>())
    );
    auto const result = airport.find_max_cost_tariff();

    if(result) {
      auto const* top = result.value();
      std::println(
        "Max cost destination: {} ({:.2f} RUB)",
        top->destination(),
        top->effective_price().rubles()
      );
    }
    else
      std::println(stderr, "System error: {}", static_cast<int>(result.error()));

    std::println("\ndoing invalid things on purpose...");
    Tariff invalid("", Money::from_decimal(-10), std::make_unique<StandardPricing>());
  } catch(std::exception const& e) {
    std::println(stderr, "exception caught: {}", e.what());
  }
  return 0;
}
