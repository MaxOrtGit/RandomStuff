// tried to make the vector of types instead be a vector for pair<type, formatter<T>> but it didn't work

#include <print>
#include <string_view>
#include <format>
#include <vector>
#include <ranges> // find
#include <iostream>
#include <utility> // pair

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

struct base_formatter {
};

template <typename T>
struct new_formatter : base_formatter {
  formatter<T> f;
  constexpr new_formatter(formatter<T> f) : f(f) {}
};



// {} || {:} print all
// letters tell what to print
// {dis} print double, int, string
// {i[format]} print int with format
// {d[format]is[format]} prints double with format, int default, string with format
template <>
struct formatter<Object> {
private:
  enum class type { i, d, c, s, none};

  vector<pair<type, unique_ptr<base_formatter>>> types;

  static constexpr bool in_vector(auto const& v, type e)
  {
    for (auto i = 0; i < v.size(); i++)
    {
      if (v[i].first == e) return true;
    }
    return false;
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
      {
        if (in_vector(types, type::i)) throw format_error("Duplicate type");
        base_formatter* nf = static_cast<base_formatter*>(new new_formatter<int>(parseFormatter<int>(ctx, it)));
        pair<type, unique_ptr<base_formatter>> p(type::i, unique_ptr<base_formatter>(nf));
        types.push_back(p);
        break;
      }
      case 'd':
      {
        if (in_vector(types, type::d)) throw format_error("Duplicate type");
        base_formatter* nf = static_cast<base_formatter*>(new new_formatter<double>(parseFormatter<double>(ctx, it)));
        pair<type, unique_ptr<base_formatter>> p(type::d, unique_ptr<base_formatter>(nf));
        types.push_back(p);
        break;
      }
      case 'c':
      {
        if (in_vector(types, type::c)) throw format_error("Duplicate type");
        base_formatter* nf = static_cast<base_formatter*>(new new_formatter<char>(parseFormatter<char>(ctx, it)));
        pair<type, unique_ptr<base_formatter>> p(type::c, unique_ptr<base_formatter>(nf));
        types.push_back(p);
        break;
      }
      case 's':
      {
        if (in_vector(types, type::s)) throw format_error("Duplicate type");
        base_formatter* nf = static_cast<base_formatter*>(new new_formatter<string_view>(parseFormatter<string_view>(ctx, it)));
        pair<type, unique_ptr<base_formatter>> p(type::s, unique_ptr<base_formatter>(nf));
        types.push_back(p);
        break;
      }
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
      switch (types[i].first)
      {
      case type::i:
      {
        out = format_to(out, "i=");
        new_formatter<int>* nf = static_cast<new_formatter<int>*>(types[i].second.get());
        out = nf->f.format(obj.i, ctx);
        break;
      }
      case type::d:
      {
        out = format_to(out, "d=");
        new_formatter<double>* nf = static_cast<new_formatter<double>*>(types[i].second.get());
        out = nf->f.format(obj.d, ctx);
        break;
      }
      case type::c:
      {
        out = format_to(out, "c=");
        new_formatter<char>* nf = static_cast<new_formatter<char>*>(types[i].second.get());
        out = nf->f.format(obj.c, ctx);
        break;
      }
      case type::s:
      {
        out = format_to(out, "s=");
        new_formatter<string_view>* nf = static_cast<new_formatter<string_view>*>(types[i].second.get());
        out = nf->f.format(obj.s, ctx);
        break;
      }
      }
      if (i < types.size() - 1) out = format_to(out, ", ");
    }
    return out;
  }
};



int main() {
  print("MSVC version {}\n", _MSC_VER); // needs to be 1937 or higher

  Object obj{11, 2.0, '3', "four"};
  std::cout << format("{:s[]di[_^10x]}-", obj);
}