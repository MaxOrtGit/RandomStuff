#include <experimental/meta>
#include <optional>
#include <iostream>
#include <algorithm>


namespace meta = std::meta;

// Helper function to get an attribute
// Gets the first instance of class type from attributes
template <typename Attribute>
consteval std::optional<Attribute> GetAttribute(meta::info info)
{
  // Loops through all attributes
  for (auto ann : meta::annotations_of(info))
  {
    // If attribute of return it
    if (meta::type_of(ann) == ^^Attribute)
      return meta::extract<Attribute>(ann);
  }
  return {};
}


// The Attribute
struct Property
{
  // Getter or Setter
  enum { Getter, Setter } type;
  // Name of variable
  std::string_view name;
};

// Metafunction to create proxy classes
consteval void Properties(meta::info cls)
{
  // name, body, add to list
  using ProxyInfo = std::tuple<std::string_view, meta::info, bool>;

  std::vector<ProxyInfo> proxies;

  // ----- Find Getter and Setter functons and store functions -----
  for (auto mem : meta::members_of(cls))
  {
    // Insert getter 
    if (auto property = GetAttribute<Property>(mem))
    {
      ProxyInfo proxyInfo { property->name, ^^{}, true};

      // Check if already in list
      auto valIt = std::find_if(proxies.begin(), proxies.end(), [&](auto val) 
      {
        return std::get<0>(val) == property->name;
      });

      // Set reference to new info or old
      ProxyInfo& valInfo = valIt == proxies.end() ? proxyInfo : *valIt;

      // Tokens of function
      meta::info function = ^^{};
      
      if (property->type == Property::Getter)
      {
        // Conversion to return type of getter
        function = ^^{ \tokens(function)
          operator [:meta::return_type_of(\(mem)):]()
          {
            return parent.[:\(mem):]();
          }
        };
      }
      else if (property->type == Property::Setter)
      {
        // = operator with val of first param of setter
        function = ^^{
          auto operator =([:meta::type_of(meta::parameters_of(\(mem))[0]):] val)
          {
            return parent.[:\(mem):](val);
          }
        };
      }

      // Add function to Proxy info
      std::get<1>(valInfo) = ^^{ \tokens(std::get<1>(valInfo))
        \tokens(function)
      };
      

      // Add to proxy list if new
      if (std::get<2>(valInfo))
      {
        std::get<2>(valInfo) = false;
        proxies.push_back(valInfo);
      }
    }
  }


  // ----- Set variable to Proxy classes -----
  // Body of class
  meta::info body = ^^{};
  
  // Fill with proxy classes
  for (auto [name, proxyBody, _] : proxies)
  {

    // No name type with ref to parent, and functions
    body = ^^{\tokens(body)
      struct
      {
        [:\(cls):]& parent;
        \tokens(proxyBody)
      } \id(name){*this};
    };
  }

  
  // ----- Create new class -----
  // remove_volatile_t is just to fix a compiler bug because 
  //   public typename[:\(cls):] doesnt work (expected an identifier)
  queue_injection(^^{
    // New Class : Old Class
    class \id(meta::identifier_of(cls)) : public std::remove_volatile_t<typename[:\(cls):]>
    {
      public:
        \tokens(body)
    };
  });  
}



// ----------------- Overloading -----------------
namespace __prototype
{
  // [[@Properties]] would be AMAZING syntax
  class Overloading
  {
    float f_val;

    // Int getters and setter
    [[=Property(Property::Getter, "var")]]
    int VarGetterInt()
    {
      std::cout << "getting int " << f_val << "\n";
      return static_cast<int>(f_val);
    }
    [[=Property(Property::Setter, "var")]]
    void VarSetterInt(int val)
    {
      f_val = static_cast<float>(val);
      std::cout << "setting int " << f_val << "\n";
    }
    
    // Bool getters and setters
    [[=Property(Property::Getter, "var")]]
    bool VarGetterBool()
    {
      std::cout << "getting bool " << f_val << "\n";
      return static_cast<int>(f_val);
    }
    [[=Property(Property::Setter, "var")]]
    void VarSetterBool(bool val)
    {
      f_val = static_cast<float>(val);
      std::cout << "setting bool " << f_val << "\n";
    }
  };
}

// Metafunc because you cant look at members inside
// I use inheritance for existing members but I feel there should be a better solution
consteval { Properties(^^__prototype::Overloading); }


void TestOverloading()
{
  Overloading obj;
  obj.var = 10;
  std::cout << static_cast<int>(obj.var) << "\n";

  obj.var = true;
  std::cout << static_cast<bool>(obj.var) << "\n";
}




// ----------------- Lazy -----------------
namespace __prototype
{
  // [[@Properties]] would be AMAZING syntax
  struct Item
  {
    float cost;
    int quantity;
    Item(float cost, int quantity) : cost(cost), quantity(quantity) {}
    
    void print()
    {
      // Need to call internal Getters/Setters
      std::cout << "Cost: $" << cost << ", Quanitity: " << quantity << 
                   ", Discount: " << DiscountGetter() << "%, Total: $" << TotalGetter() << 
                   ", (priceMod: " << priceMod << ")\n";
    }



    private:

    // ----- Discount ----- (to percent)
    float priceMod = 1.0;
    [[=Property(Property::Getter, "discount")]]
    float DiscountGetter()
    {
      return (1 - priceMod) * 100;
    }
    [[=Property(Property::Setter, "discount")]]
    void DiscountSetter(float discountPercent)
    {
      // Modifies the discount
      priceMod = 1 - discountPercent / 100;
    }


    // ----- Total -----
    [[=Property(Property::Getter, "total")]]
    float TotalGetter()
    {
      return cost * quantity * priceMod;
    }
    [[=Property(Property::Setter, "total")]]
    void TotalSetter(float totalValue)
    {
      // Modifies the discount
      priceMod = totalValue / (cost * quantity);
    }
    
  };
}

consteval { Properties(^^__prototype::Item); }


void TestLazy()
{
  // Because I use inheritance it needs to be initalized like this
  // Having to code this manually def makes metaclasses/class decorators a harder sell
  Item gold({12, 3});
  gold.print();

  // 25% discount
  gold.discount = 25;
  gold.print();

  // Capping total
  gold.total = 30;
  gold.print();
}



int main()
{
  std::cout << "------ Overlaoding ------\n";
  TestOverloading();
  std::cout << "\n------ Lazy ------\n";
  TestLazy();
}

