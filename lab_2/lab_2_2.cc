#include <cstdint>
#include <print>
#include <string>
#include <vector>
#include <string_view>
#include <iostream>
#include <limits>
#include <numeric>

#include "../money.h"

namespace lab_2
{
  enum class TicketClass
  {
    Economy,
    Business,
    First
  };

  [[nodiscard]] std::string_view to_string(TicketClass tc)
  {
    switch(tc) {
      case TicketClass::Economy: return "Economy";
      case TicketClass::Business: return "Business";
      case TicketClass::First: return "First";
      default: return "Unknown";
    }
  }

  class Passenger
  {
    std::string m_name;
    std::string m_passport;

   public:
    Passenger(std::string_view name, std::string_view const passport)
      : m_name(name)
      , m_passport(passport)
    {}

    ~Passenger() = default;

    [[nodiscard]] std::string_view name() const { return this->m_name; }

    [[nodiscard]] std::string_view passport() const { return this->m_passport; }
  };

  class Tariff
  {
    std::string m_dest;
    Money m_base_price;
    TicketClass m_class;

   public:
    Tariff(std::string_view const d, Money const p, TicketClass const c)
      : m_dest(d)
      , m_base_price(p)
      , m_class(c)
    {}

    ~Tariff() = default;

    [[nodiscard]] std::string_view destination() const { return this->m_dest; }

    [[nodiscard]] TicketClass category() const { return this->m_class; }

    [[nodiscard]] Money price() const { return this->m_base_price; }
  };

  class Ticket
  {
    Passenger m_passenger;
    Tariff m_tariff;

   public:
    Ticket(Passenger p, Tariff t)
      : m_passenger(std::move(p))
      , m_tariff(std::move(t))
    {}

    [[nodiscard]] Money price() const { return this->m_tariff.price(); }

    [[nodiscard]] Passenger const& passenger() const { return this->m_passenger; }
  };

  class BookingSystem
  {
    std::vector<Tariff> m_available_tariffs;
    std::vector<Ticket> m_sold_tickets;

   public:
    ~BookingSystem() = default;

    void add_tariff(Tariff t) { this->m_available_tariffs.push_back(std::move(t)); }

    void register_sale(Ticket t) { this->m_sold_tickets.push_back(std::move(t)); }

    [[nodiscard]] std::vector<Tariff> const& tariffs() const { return this->m_available_tariffs; }

    [[nodiscard]] std::vector<Ticket> const& tickets() const { return this->m_sold_tickets; }

    [[nodiscard]] Money calculate_total_revenue() const
    {
      return std::accumulate(
        this->m_sold_tickets.begin(),
        this->m_sold_tickets.end(),
        Money(0),
        [](Money const sum, Ticket const& t) { return sum + t.price(); }
      );
    }
  };

  class Menu
  {
   public:
    Menu()
    {
      std::println(
        "1. Add new Tariff | 2. Show all Tariffs | 3. Buy a Ticket | 4. Show Sales Report | 0. Exit"
      );
    }

    [[nodiscard]] std::string read_string(std::string_view const prompt) const
    {
      auto input = std::string();
      while(true) {
        std::print("{}: ", prompt);
        if(std::getline(std::cin >> std::ws, input) && ! input.empty())
          return input;
        std::println("Error: Input cannot be empty.");
        this->clear_cin();
      }
    }

    [[nodiscard]] double read_double(std::string_view const prompt, double const min_val) const
    {
      auto val = .0;
      while(true) {
        std::print("{}: ", prompt);
        if(std::cin >> val && val >= min_val)
          return val;
        std::println("Error: Enter a valid number (min: {}).", min_val);
        this->clear_cin();
      }
    }

    [[nodiscard]] int read_int(std::string_view const prompt, int const min, int const max) const
    {
      auto val = 0;
      while(true) {
        std::print("{}: ", prompt);
        if(std::cin >> val && val >= min && val <= max)
          return val;
        std::println("Error: Choice out of range ({}-{}).", min, max);
        this->clear_cin();
      }
    }

   private:
    void clear_cin() const
    {
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
  };
}  // namespace lab_2

int main()
{
  using namespace lab_2;
  BookingSystem system;

  while(true) {
    auto const menu = Menu();
    auto const choice = menu.read_int("Select action", 0, 4);
    if(choice == 0)
      break;

    switch(choice) {
      case 1: {
        auto const dest = menu.read_string("Destination");
        auto const price_val = menu.read_double("Base price", 0.0);
        std::println("Class: 0 - Economy, 1 - Business, 2 - First");
        auto const class_idx = menu.read_int("Select class", 0, 2);

        system.add_tariff(
          Tariff(dest, Money::from_decimal(price_val), static_cast<TicketClass>(class_idx))
        );
        break;
      }
      case 2: {
        auto& tariffs = system.tariffs();
        for(auto i = 0ull; i < tariffs.size(); ++i) {
          std::println(
            "{}. {} | {} | {:.2f} RUB",
            i,
            tariffs[i].destination(),
            to_string(tariffs[i].category()),
            tariffs[i].price().rubles()
          );
        }
        break;
      }
      case 3: {
        auto const& tariffs = system.tariffs();
        if(tariffs.empty()) {
          std::println("No tariffs available!");
          break;
        }

        auto const
          t_idx = menu.read_int("Select tariff index", 0, static_cast<int>(tariffs.size() - 1));
        auto const name = menu.read_string("Passenger full name");
        auto const passport = menu.read_string("Passport number");

        system.register_sale(Ticket(Passenger(name, passport), tariffs[t_idx]));
        std::println("Ticket purchased successfully!");
        break;
      }
      case 4: {
        std::println("Tickets sold: {}", system.tickets().size());
        std::println("Total Revenue: {:.2f} RUB", system.calculate_total_revenue().rubles());
        break;
      }
    }
  }

  return 0;
}
