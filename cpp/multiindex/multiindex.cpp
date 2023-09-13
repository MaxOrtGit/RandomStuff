#include "Classes.h"
#include "boost/multi_index_container.hpp"
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/member.hpp>
#include <memory>
#include <typeindex>

// assign boost::multi_index to BM
namespace BM = boost::multi_index;

template <typename T>
using type_pair = std::pair<std::type_index, std::unique_ptr<T>>;

template <typename T>
struct type_id_tag {};

template <typename T>
using MultiIndexList = boost::multi_index_container<type_pair<T>,
                                   BM::indexed_by<
                                     BM::sequenced<>,
                                     BM::ordered_unique<BM::tag<type_id_tag<T>>,  BOOST_MULTI_INDEX_MEMBER(type_pair<T>, std::type_index, first)>
                                   >>;

inline static MultiIndexList<BaseClass> instances;


template <typename T>
BaseClass* GetClass() {
    auto it = instances.get<type_id_tag<BaseClass>>().find(std::type_index(typeid(T)));
    if (it != instances.get<type_id_tag<BaseClass>>().end()) {
       return it->second.get();
    }
    return nullptr;
}

int main()
{
  instances.emplace_back(std::type_index(typeid(ClassA)), std::make_unique<ClassA>());
  instances.emplace_back(std::type_index(typeid(ClassD)), std::make_unique<ClassD>());
  instances.emplace_back(std::type_index(typeid(ClassB)), std::make_unique<ClassB>());
  instances.emplace_back(std::type_index(typeid(ClassC)), std::make_unique<ClassC>());
  
  for (const auto& instance : instances) {
    std::cout << instance.first.name() << std::endl;   
  }

  std::cout << "GetClass<ClassA>() = " << GetClass<ClassA>() << std::endl;
  
  return 0;
}


