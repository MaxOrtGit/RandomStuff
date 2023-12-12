#include <string_view>
#include <vector>
#include <span>
#include <optional>

namespace std::meta {
  struct info {};
  
  consteval auto is_public(info r) -> bool;
  consteval auto is_protected(info r) -> bool;
  consteval auto is_private(info r) -> bool;
  consteval auto is_accessible(info r) -> bool;
  consteval auto is_virtual(info r) -> bool;
  consteval auto is_deleted(info entity) -> bool;
  consteval auto is_defaulted(info entity) -> bool;
  consteval auto is_explicit(info entity) -> bool;
  consteval auto is_override(info entity) -> bool;
  consteval auto is_pure_virtual(info entity) -> bool;
  consteval auto has_static_storage_duration(info r) -> bool;

  consteval auto is_nsdm(info entity) -> bool;
  consteval auto is_base(info entity) -> bool;
  consteval auto is_namespace(info entity) -> bool;
  consteval auto is_function(info entity) -> bool;
  consteval auto is_static(info entity) -> bool;
  consteval auto is_variable(info entity) -> bool;
  consteval auto is_type(info entity) -> bool;
  consteval auto is_alias(info entity) -> bool;
  consteval auto is_incomplete_type(info entity) -> bool;
  consteval auto is_template(info entity) -> bool;
  consteval auto is_function_template(info entity) -> bool;
  consteval auto is_variable_template(info entity) -> bool;
  consteval auto is_class_template(info entity) -> bool;
  consteval auto is_alias_template(info entity) -> bool;
  consteval auto has_template_arguments(info r) -> bool;
    
  consteval auto name_of(info r) -> string_view;
  consteval auto display_name_of(info r) -> string_view;
  consteval auto type_of(info r) -> info;
  consteval auto parent_of(info r) -> info;
  consteval auto entity_of(info r) -> info;
  consteval auto template_of(info r) -> info;
  consteval auto template_arguments_of(info r) -> vector<info>;
  template<typename ...Fs>
    consteval auto members_of(info class_type, Fs ...filters) -> vector<info>;
  template<typename ...Fs>
    consteval auto nonstatic_data_members_of(info class_type, Fs ...filters) -> vector<info> {
      return members_of(class_type, is_nsdm, filters...);
    }
  template<typename ...Fs>
    consteval auto bases_of(info class_type, Fs ...filters) -> vector<info> {
      return members_of(class_type, is_base, filters...);
    }
  template<typename ...Fs>
    consteval auto enumerators_of(info class_type, Fs ...filters) -> vector<info>;
  template<typename ...Fs>
    consteval auto subobjects_of(info class_type, Fs ...filters) -> vector<info>;
  consteval auto substitute(info templ, span<info const> args) -> info;
  template<typename T> consteval auto entity_ref(info var_or_func) -> T&;
  template<typename T> consteval auto value_of(info constant_expr) -> T;
  template<typename T> consteval auto pointer_to_member(info member) -> T;
  
  auto test_type(info templ, info type) -> bool;

  auto test_types(info templ, span<info const> types) -> bool;

  template<typename T> consteval auto reflect_value(T value) -> info;
  struct nsdm_field_args {
    optional<string_view> name;
    optional<int> alignment;
    optional<int> width;
  };

  consteval auto nsdm_description(info type, nsdm_field_args args = {}) -> info;

  consteval auto synth_struct(span<info const>) -> info;
  consteval auto synth_union(span<info const>) -> info;

  consteval auto offset_of(info entity) -> size_t;
  consteval auto size_of(info entity) -> size_t;

  consteval auto bit_offset_of(info entity) -> size_t;
  consteval auto bit_size_of(info entity) -> size_t;
}


consteval std::meta::info GetFirstMember(const std::meta::info& objectInfo)
{
  return std::meta::nonstatic_data_members_of(objectInfo)[0];
}

consteval size_t GetMemberCount(const std::meta::info& objectInfo)
{
  return std::meta::nonstatic_data_members_of(objectInfo).size();
}