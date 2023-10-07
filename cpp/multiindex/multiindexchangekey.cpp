#include "boost/multi_index_container.hpp"
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/member.hpp>
#include <memory>
#include <iostream>

// assign boost::multi_index to BM
namespace BM = boost::multi_index;

using pair = std::pair<float, int>;

struct zLevel_tag {};

using MultiIndexList = boost::multi_index_container<pair,
                                   BM::indexed_by<
                                     BM::ordered_non_unique<BM::tag<zLevel_tag>,  BOOST_MULTI_INDEX_MEMBER(pair, float, first)>
                                   >>;

inline static MultiIndexList instances;


int main()
{
  instances.emplace(1.0f, 2);
  instances.emplace(-1.4f, 3);
  instances.emplace(1.2f, 4);
  instances.emplace(2.0f, 5);
  instances.emplace(-1.1f, 6);
  instances.emplace(2.0f, 7);
  instances.emplace(4.5f, 8);
  instances.emplace(-2.2f, 9);
  instances.emplace(-1.0f, 10);

  auto& zLevel_index = instances.get<zLevel_tag>();

  // call modify on every element
  using Iterator = MultiIndexList::index<zLevel_tag>::type::iterator;

  std::vector<Iterator> view;
  view.reserve(zLevel_index.size());
  for (Iterator it = zLevel_index.begin(); it != zLevel_index.end(); ++it){
    view.push_back(it);
  }

  for (auto& it : view) {
    zLevel_index.modify(it, [](pair& p) { 
      p.first -= 0.5f; 
    });
  }

  // print out the zLevels
  for (const auto& instance : zLevel_index) 
  {
    std::cout << instance.first << std::endl;   
  }

  return 0;
}


