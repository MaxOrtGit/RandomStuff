#include <print>
#include <string>
#include <string_view>
#include <format>
#include <vector>
#include <variant>
#include <tuple>

// it and ctx are modified
template <typename T, typename CharT>
constexpr std::formatter<T, CharT> parseFormatter(std::basic_format_parse_context<CharT>& ctx, auto& it)
{
  if (*it != '[') return std::formatter<T, CharT>();
  
  int timer = 0;
  auto end_bracket = it + 1;

  int scope = 1; // ++ for [, -- for ]
  // looks for matching ]
  while (true)
  {
    if (*end_bracket == '[') scope++;
    else if (*end_bracket == ']') 
    {
      scope--;
      if (scope == 0) break;
    }
    end_bracket++;
  }
  
  std::basic_format_parse_context<CharT> ctx2(std::basic_string_view(it + 1, end_bracket));
  std::formatter<T, CharT> f;
  f.parse(ctx2);
  it = end_bracket + 1;
  
  return f;
}

template <typename CharT>
constexpr void GoToNext(std::basic_format_parse_context<CharT>& ctx, auto& it)
{
  if (*it != '[') return ;
  
  int timer = 0;
  auto end_bracket = it + 1;

  int scope = 1; // ++ for [, -- for ]
  // looks for matching ]
  while (true)
  {
    if (*end_bracket == '[') scope++;
    else if (*end_bracket == ']') 
    {
      scope--;
      if (scope == 0) break;
    }
    end_bracket++;
  }
  
  it = end_bracket + 1;
  
  return ;
}

template <typename CharT, typename T>
std::formatter<T, CharT> make_formatter(T val, std::string_view format) {
  std::basic_format_parse_context<CharT> ctx(format);
  std::formatter<T, CharT> f;
  f.parse(ctx);
  return f;
}

// {} || {:} print all
// letters tell what to print
// {dis} print double, int, string
// {i[format]} print int with format
// {d[format]is[format]} prints double with format, int default, string with format
template <typename CharT, typename ...Ts>
struct std::formatter<std::tuple<Ts...>, CharT> {
private:
  using FormatterVariant = std::variant<formatter<Ts, CharT>...>;
  using InputTuple = std::tuple<size_t, size_t, FormatterVariant>; // order, tuple type index, formatter
  std::vector<InputTuple> input_formats;
  using OutputTuple = std::tuple<size_t, std::string>; // order, output string
  std::vector<std::string> output_strings;
  
  std::string connector = ", ";
  std::string prefix = "Tuple: ";

  template <typename T, std::size_t I>
  constexpr void ParseType(auto& furthest, auto const& start, auto& ctx)
  {
    auto it = start;
    const auto end = ctx.end();
    size_t order = 0;
    while (it != end && *it != '}')
    {
      // get number from it (will end in a ',', '[', or '}')
      size_t index = 0;
      while ((it != end && *it != ',' && *it != '[' && *it != '}') && *it >= '0' && *it <= '9')
      {
        index *= 10;
        index += *it - '0';
        it++;
      }
      if (index == I)
      {
        FormatterVariant fv(std::in_place_index<I>, parseFormatter<T>(ctx, it));
        InputTuple inputTuple = {order, index, fv};
        input_formats.push_back(inputTuple);
      }
      else
        GoToNext<CharT>(ctx, it);

      if (it == end || *it == '}')
      { 
        furthest = it;
        break;
      }
      if (*it == ',') it++;
      else throw std::format_error("Invalid format string, no comma");
      if (*it == ' ') it++;
      order++;
    }
    furthest = it;
  }

  
  // require T to be a tuple with type Ts
  template <size_t I = 0>
  constexpr void ParseAllTypes(auto& furthest, auto const& start, auto& ctx)
  {
    if constexpr (I >= sizeof...(Ts)) return;
    else {
      ParseType<std::tuple_element_t<I, std::tuple<Ts...>>, I>(furthest, start, ctx);
      ParseAllTypes<I + 1>(furthest, start, ctx);
    }
  }

  
  template <typename T, std::size_t I>
  constexpr bool OutputValAtOrder(auto const& obj, auto& ctx, auto& out, size_t& order)
  {
    //input_formats: order, tuple type index, formatter
    for (auto const& input_format : input_formats) {
      if (std::get<1>(input_format) == I && std::get<0>(input_format) == order) {
        out = std::get<I>(std::get<2>(input_format)).format(std::get<I>(obj), ctx);
        if (order != input_formats.size() - 1)
      out = std::format_to(out, "{}", connector);
        return true;
      }
    }
    return false;
  }

  template <size_t I = 0>
  constexpr void OutputForFormats(auto const& obj, auto& ctx, auto& out, size_t& order)
  {
    if constexpr (I >= sizeof...(Ts)) return;
    else {
      bool found = OutputValAtOrder<std::tuple_element_t<I, std::tuple<Ts...>>, I>(obj, ctx, out, order);
      if (found) return;
      OutputForFormats<I + 1>(obj, ctx, out, order);
    }
  }
  
  template <typename T, size_t I>
  constexpr void OutputVal(auto const& obj, auto& ctx, auto& out)
  {
    out = std::format_to(out, "{}", std::get<I>(obj));
    if (I != sizeof...(Ts) - 1)
      out = std::format_to(out, "{}", connector);
  }

  template <size_t I = 0>
  constexpr void OutputForNoFormats(auto const& obj, auto& ctx, auto& out)
  {
    if constexpr (I >= sizeof...(Ts)) return;
    else 
    {
      OutputVal<std::tuple_element_t<I, std::tuple<Ts...>>, I>(obj, ctx, out);
      OutputForNoFormats<I + 1>(obj, ctx, out);
    }
  }



public:
  constexpr auto parse(auto& ctx) {
    auto it = ctx.begin();
    const auto end = ctx.end();
    if (it == end || *it == '}') {
      return it;
    }
    if (*it == '(')
    {
      prefix.clear();
      // text between () is the prefix
      it++;
      while (it != end && *it != ')') {
        prefix.push_back(*it);
        it++;
      }
      it++;
    }
    {
      connector.clear();
      // text between [] is the connector
      it++;
      while (it != end && *it != ']') {
        connector.push_back(*it);
        it++;
      }
      it++;
    }
    if (*it == '[')
    {
      connector.clear();
      // text between [] is the connector
      it++;
      while (it != end && *it != ']') {
        connector.push_back(*it);
        it++;
      }
      it++;
    }
    if (it == end || *it == '}') {
      return it;
    }
    auto furthest = ctx.begin();
    ParseAllTypes(furthest, it, ctx);
    return furthest;
  }


  auto format(const tuple<Ts...>& obj, auto& ctx) {
    auto out = std::format_to(ctx.out(), "{}", prefix);
    if (input_formats.size() == 0) {
      OutputForNoFormats(obj, ctx, out);
      return out;
    }
    for (size_t i = 0; i < input_formats.size(); i++) {
      OutputForFormats(obj, ctx, out, i);
    }
    return out;
  }
};



int main() {
  std::print("MSVC version {}\n", _MSC_VER); // needs to be 1937 or higher

  std::tuple<int, float, char, std::string_view, float, float> obj{11, 2.0, '%', "four", 1.2, 3.4};
  std::println("-{:()[ ]}-", obj);
  std::println("-{:[ - ]3[], 0[_^10x], 1, 4, 5}-", obj);
}