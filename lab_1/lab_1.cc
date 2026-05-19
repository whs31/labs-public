#include <cstdint>
#include <cstddef>

#include <print>
#include <string>
#include <string_view>
#include <vector>

namespace lab_1 {
  class Money {
   public:
    constexpr Money(std::int64_t cents = 0)
      : m_cents(cents)
    {}

    [[nodiscard]] constexpr static Money from_decimal(double const rubles) {
      return Money(static_cast<std::int64_t>(rubles * 100.0 + .5));
    }

    [[nodiscard]] constexpr std::int64_t cents() const {
      return this->m_cents;
    }

    [[nodiscard]] constexpr double rubles() const {
      return this->m_cents / 100.;
    }

    Money operator*(std::int64_t const count) const {
      return Money(this->m_cents * count);
    }

   private:
    std::int64_t m_cents;
  };

  class Airport {
   public:
    Airport(
      std::string_view const name,
      Money const ticket_price,
      std::size_t const total_ticket_count
    )
      : m_name(std::string(name))
      , m_ticket_price(ticket_price)
      , m_total_ticket_count(total_ticket_count)
      , m_sold_ticket_count(0ull)
    {}

    [[nodiscard]] std::string_view name() const { return this->m_name; }
    void set_name(std::string_view const new_name) { this->m_name = std::string(new_name); }

    [[nodiscard]] Money ticket_price() const { return this->m_ticket_price; }
    void set_ticket_price(Money const new_price) { this->m_ticket_price = new_price; }

    [[nodiscard]] std::size_t total_ticket_count() const { return this->m_total_ticket_count; }
    void set_total_ticket_count(std::size_t const new_count) { this->m_total_ticket_count = new_count; }

    [[nodiscard]] std::size_t sold_ticket_count() const { return this->m_sold_ticket_count; }
    void set_sold_ticket_count(std::size_t const new_count) { this->m_sold_ticket_count = new_count; }

    void set_secret(std::string_view const secret) {
      this->m_secret = std::string(secret);
    }

    Airport& sell(std::size_t const count) {
      if(this->m_sold_ticket_count + count > this->m_total_ticket_count) {
        std::println(
          stderr,
          "Error: Not enough tickets! Requested: {}, Available: {}",
          count,
          this->m_total_ticket_count - this->m_sold_ticket_count
        );
        return *this;
      }
      this->m_sold_ticket_count += count;
      return *this;
    }

    [[nodiscard]] Money total_revenue() const {
      return this->m_ticket_price * this->m_sold_ticket_count;
    }

   private:
    std::string m_name;
    Money m_ticket_price;
    std::size_t m_total_ticket_count;
    std::size_t m_sold_ticket_count;
    std::string m_secret;
  };
}

int main() {
  using namespace lab_1;

  auto const price = Money::from_decimal(10'999.99);
  auto airport = Airport("Pulkovo", price, 1'000);

  airport.set_secret("very hidden secret, no one can read");
  airport
    .sell(100)
    .sell(200)
    .sell(300);

  std::println("\n---");
  std::println("Airport: {}", airport.name());
  std::println("Revenue: {:.2f} RUB", airport.total_revenue().rubles());

  // в С++ невозможно сделать по-настоящему скрытые поля, которые будут доступны только для записи:
  std::println(
    "Secret: {}",
    *reinterpret_cast<std::string*>(
      reinterpret_cast<unsigned char*>(&airport) + sizeof(std::string) + sizeof(Money) + sizeof(std::size_t) * 2)
  );

  return 0;
}
