
#include <print>
#include <string_view>
#include <format>
#include <vector>
#include <array>
#include <iterator>
#include <utility> // pair
#include <ranges> // find

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
constexpr formatter<T> parseFormatter(basic_format_parse_context<CharT>& ctx, auto& it)
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
template <>
struct formatter<Object> {
private:
  enum class type { i, d, c, s, none};

  type type1 = type::none;
  vector<type> types;

  formatter<int> int_formatter;
  formatter<double> double_formatter;
  formatter<char> char_formatter;
  formatter<string_view> string_formatter;

  static constexpr bool in_vector(auto const& v, type e)
  {
    return ranges::find(v, e) != v.end();
  }


public:
  constexpr auto parse(auto& ctx) {
    auto it = ctx.begin();
    const auto end = ctx.end();
    if (it == end || *it == '}') {
      return it;
    }
    int i = 0;
    while  (it != end && *it != '}')
    {
      switch (*it++)
      {
      case 'i':
        if (in_vector(types, type::i)) throw format_error("Duplicate type");
        types.push_back(type::i);
        int_formatter = parseFormatter<int>(ctx, it);
        break;
      case 'd':
        if (in_vector(types, type::d)) throw format_error("Duplicate type");
        types.push_back(type::d);
        double_formatter = parseFormatter<double>(ctx, it);
        break;
      case 'c':
        if (in_vector(types, type::c)) throw format_error("Duplicate type");
        types.push_back(type::c);
        char_formatter = parseFormatter<char>(ctx, it);
        break;
      case 's':
        if (in_vector(types, type::s)) throw format_error("Duplicate type");
        types.push_back(type::s);
        string_formatter = parseFormatter<string_view>(ctx, it);
        break;
      default:
        throw format_error("Unknown type");
      }
      i++;
    }
    return it;
  }

  auto format(const Object& obj, auto& ctx) {
    auto out = format_to(ctx.out(), "Object: ");
    for (auto i = 0; i < types.size(); i++)
    {
      switch (types[i])
      {
      case type::i:
        out = format_to(out, "i=");
        out = int_formatter.format(obj.i, ctx);
        break;
      case type::d:
        out = format_to(out, "d=");
        out = double_formatter.format(obj.d, ctx);
        break;
      case type::c:
        out = format_to(out, "c=");
        out = char_formatter.format(obj.c, ctx);
        break;
      case type::s:
        out = format_to(out, "s=");
        out = string_formatter.format(obj.s, ctx);
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
  println("{:s[]di[_^10x]}-", obj);
}