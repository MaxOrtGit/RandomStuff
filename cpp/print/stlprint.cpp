#include <print>
#include <string_view>
#include <format>
#include <vector>
#include <iostream>
#include <variant>

using namespace std;

struct Object {
  private:
    int i;
    double d;
    char c;
    string_view s;
    friend struct formatter<Object>;

  public:
    Object(int i, double d, char c, string_view s) : i(i), d(d), c(c), s(s) {}

};


// it and ctx are modified
template <typename T, typename CharT>
constexpr formatter<T, CharT> parseFormatter(basic_format_parse_context<CharT>& ctx, auto& it)
{
  if (*it != '[') return formatter<T>();
  
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

  basic_format_parse_context<CharT> ctx2(basic_string_view(it + 1, end_bracket));
  formatter<T> f;
  f.parse(ctx2);
  it = end_bracket + 1;
  
  return f;
}


// {} || {:} print all
// letters tell what to print
// {dis} print double, int, string
// {i[format]} print int with format
// {d[format]is[format]} prints double with format, int default, string with format
template <typename CharT>
struct formatter<Object, CharT> {
private:
  enum class type { i, d, c, s, none};
  using FormatterVariant = std::variant<formatter<int, CharT>, formatter<double, CharT>, formatter<char, CharT>, formatter<string_view, CharT>>;

  vector<pair<type, FormatterVariant>> types;


public:
  constexpr auto parse(auto& ctx) {
    auto it = ctx.begin();
    const auto end = ctx.end();
    if (it == end || *it == '}') {
      return it;
    }
    while  (it != end && *it != '}')
    {
      switch (*it++)
      {
      case 'i':
        types.emplace_back(type::i, parseFormatter<int>(ctx, it));
        break;
      case 'd':
        types.emplace_back(type::d, parseFormatter<double>(ctx, it));
        break;
      case 'c':
        types.emplace_back(type::c, parseFormatter<char>(ctx, it));
        break;
      case 's':
        types.emplace_back(type::s, parseFormatter<string_view>(ctx, it));
        break;
      default:
        throw format_error("Unknown type");
      }
    }
    return it;
  }

  auto format(const Object& obj, auto& ctx) {
    auto out = format_to(ctx.out(), "Object: ");
    if (types.empty())
    {
      out = format_to(out, "i={}, d={}, c={}, s={}", obj.i, obj.d, obj.c, obj.s);
      return out;
    }
    for (auto i = 0; i < types.size(); i++)
    {
      auto &type = types[i].first;
      auto &var = types[i].second;
      switch (type)
      {
      case type::i:
        out = format_to(out, "i=");
        out = get<formatter<int, CharT>>(var).format(obj.i, ctx);
        break;
      case type::d:
        out = format_to(out, "d=");
        out = get<formatter<double, CharT>>(var).format(obj.d, ctx);
        break;
      case type::c:
        out = format_to(out, "c=");
        out = get<formatter<char, CharT>>(var).format(obj.c, ctx);
        break;
      case type::s:
        out = format_to(out, "s=");
        out = get<formatter<string_view, CharT>>(var).format(obj.s, ctx);
        break;
      }
      if (i < types.size() - 1) out = format_to(out, ", ");
    }
    return out;
  }
};

int main() {
  print("MSVC version {}\n", _MSC_VER); // needs to be 1937 or higher

  Object obj{11, 2.0, '3', "four"};
  println("-{}-", obj);
  println("-{:s[]di[_^10x]i}-", obj);
}