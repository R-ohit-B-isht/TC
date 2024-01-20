#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include <assert.h>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <limits>
#include <map>

template <typename K, typename V> class interval_map {

public:
  std::map<K, V> m_map;
  // constructor associates whole range of K with val by inserting (K_min, val)
  // into the map
  explicit interval_map(V const &val) {
    m_map.insert(m_map.end(),
                 std::make_pair(std::numeric_limits<K>::lowest(), val));
  }

  // Assign value val to interval [keyBegin, keyEnd).
  // Overwrite previous values in this interval.
  // Conforming to the C++ Standard Library conventions, the interval
  // includes keyBegin, but excludes keyEnd.
  // If !( keyBegin < keyEnd ), this designates an empty interval,
  // and assign must do nothing.
  void assign(K const &keyBegin, K const &keyEnd, V const &val) {
    if (!(keyBegin < keyEnd))
      return;

    typename std::map<K, V>::iterator
        iterBegin; /*The new begin with val, can be begin()*/
    typename std::map<K, V>::iterator
        iterEnd; /*the new end of val, can be end()*/

    auto lowerKeyBegin =
        m_map.lower_bound(keyBegin); // either end() or some iter whose key is
                                     // not less than keyBegin. [1st O(logN)]
    auto upperKeyEnd = m_map.upper_bound(
        keyEnd); // some iter where keyEnd < key, or end()  [2nd O(logN)]
    auto prevKeyEnd = std::prev(upperKeyEnd);

    /*
    The next interval of the new interval starts at keyEnd if the previous value
    at keyEnd differed from val
    */
    if (!(prevKeyEnd->second == val)) {
      // prevKeyEnd is either less than the new end we are inserting, or the
      // same (no update to avoid copying from erased node)
      if (!(prevKeyEnd->first < keyEnd) && !(keyEnd < prevKeyEnd->first))
        iterEnd = prevKeyEnd;
      else
        iterEnd =
            m_map.insert_or_assign(upperKeyEnd, keyEnd, prevKeyEnd->second);
    } else {
      iterEnd = upperKeyEnd;
    }

    /*
    The new interval starts at keyBegin if the would-be previous interval has a
    different value. Previous interval is either a key in the map less than
    keyBegin, or non-existent when lower_bound is m_map.begin() The new
    interval's start is merged with previous interval, if the previous interval
    has the same value.
    */
    if (lowerKeyBegin != m_map.begin()) {
      auto prevIter =
          std::prev(lowerKeyBegin); // safe when end(), because we always have
                                    // at least one value
      if (!(prevIter->second == val)) {
        iterBegin = m_map.insert_or_assign(lowerKeyBegin, keyBegin, val);
      } else
        iterBegin = prevIter;
    } else {
      iterBegin = m_map.insert_or_assign(lowerKeyBegin, keyBegin, val);
    }

    /*
    Erase all keys between the new begin and end (excluding) so that there is
    only one value after iterBegin This is fine when iterEnd is end()
    */
    {
      auto nextIterOfBegin = std::next(
          iterBegin); // somehow msvc doesn't support if-initialization
      if (nextIterOfBegin != m_map.end()) {
        // I would be very interested in a smarter way to get rid of this part
        // without additional storage ...
        m_map.erase(nextIterOfBegin, iterEnd);
      }
    }

    ////debug - check canonical
    // for (auto iter = m_map.begin(); iter != m_map.end(); ++iter)
    //{
    //   auto next = std::next(iter);
    //   if (next != m_map.end() && iter->second == next->second)
    //   {
    //       throw;
    //   }
    // }
    // INSERT YOUR SOLUTION HERE
  }

  // look-up of the value associated with key
  V const &operator[](K const &key) const {
    return (--m_map.upper_bound(key))->second;
  }
  void show() {
    std::cout << "show" << std::endl;
    for (auto entry : m_map) {
      std::cout << entry.first << entry.second << std::endl;
    }
  }
};

TEST_CASE("CanonicalConflicts") {
  SECTION("BeginExtra with Mid") {
    interval_map<int, char> m('A');
    m.assign(1, 5, 'B');
    m.assign(0, 3, 'C');
    REQUIRE(m[0] == 'C');
    REQUIRE(m[1] == 'C');
    REQUIRE(m[2] == 'C');
    REQUIRE(m[3] == 'B');
    REQUIRE(m[4] == 'B');
    REQUIRE(m[5] == 'A');
  }

  SECTION("Before-Mid with Mid (beginHasExtra==false)") {
    interval_map<int, char> m('A');
    m.assign(2, 5, 'B');
    m.assign(0, 3, 'C');
    REQUIRE(m[0] == 'C');
    REQUIRE(m[1] == 'C');
    REQUIRE(m[2] == 'C');
    REQUIRE(m[3] == 'B');
    REQUIRE(m[4] == 'B');
    REQUIRE(m[5] == 'A');
  }

  SECTION("Mid with EndExtra") {
    interval_map<int, char> m('A');
    m.assign(1, 5, 'B');
    m.assign(3, 8, 'C');
    REQUIRE(m[0] == 'A');
    REQUIRE(m[1] == 'B');
    REQUIRE(m[2] == 'B');
    REQUIRE(m[3] == 'C');
    REQUIRE(m[4] == 'C');
    REQUIRE(m[5] == 'C');
    REQUIRE(m[6] == 'C');
    REQUIRE(m[7] == 'C');
    REQUIRE(m[8] == 'A');
  }

  SECTION("Mid with After-Mid (endHasExtra==false)") {
    interval_map<int, char> m('A');
    m.assign(1, 4, 'B');
    m.assign(6, 9, 'C');
    REQUIRE(m[0] == 'A');
    REQUIRE(m[1] == 'B');
    REQUIRE(m[2] == 'B');
    REQUIRE(m[3] == 'B');
    REQUIRE(m[4] == 'A');
    REQUIRE(m[5] == 'A');
    REQUIRE(m[6] == 'C');
    REQUIRE(m[7] == 'C');
    REQUIRE(m[8] == 'C');
    REQUIRE(m[9] == 'A');
  }
}

TEST_CASE("TrivialRange") {
  interval_map<int, char> m('A');
  m.assign(1, 10, 'B');
  REQUIRE(m[0] == 'A');
  for (int i = 1; i < 10; i++) {
    REQUIRE(m[i] == 'B');
  }
  REQUIRE(m[10] == 'A');
}
TEST_CASE("TrivialTwoRange") {
  interval_map<int, char> m('A');
  m.assign(1, 3, 'B');
  m.assign(6, 8, 'C');
  REQUIRE(m[0] == 'A');
  REQUIRE(m[1] == 'B');
  REQUIRE(m[2] == 'B');
  REQUIRE(m[3] == 'A');
  REQUIRE(m[4] == 'A');
  REQUIRE(m[5] == 'A');
  REQUIRE(m[6] == 'C');
  REQUIRE(m[7] == 'C');
  REQUIRE(m[8] == 'A');
}
TEST_CASE("OverwriteLowest") {
  interval_map<int, char> m('A');
  m.assign(std::numeric_limits<int>::lowest(), 10000, 'B');
  REQUIRE(m[0] == 'B');
  REQUIRE(m[9999] == 'B');
  REQUIRE(m[10000] == 'A');
}

TEST_CASE("Merge") {
  interval_map<int, char> m('A');
  m.assign(std::numeric_limits<int>::lowest(), 10, 'B');
  m.assign(10, 20, 'B');
  REQUIRE(m[0] == 'B');
  REQUIRE(m[10] == 'B');
  REQUIRE(m[19] == 'B');
  REQUIRE(m[20] == 'A');
}

TEST_CASE("FloatKey") {
  interval_map<float, char> m('A');
  m.assign(1.00, 5.00, 'B');

  REQUIRE(m[0.0] == 'A');
  REQUIRE(m[0.999999999] == 'B');
  REQUIRE(m[1.0] == 'B');
  REQUIRE(m[4.999] == 'B');
  REQUIRE(m[5.0] == 'A');
}

TEST_CASE("OverlappingRangeComplete") {
  interval_map<int, char> m('A');
  m.assign(3, 5, 'B');
  m.assign(1, 6, 'C');
  REQUIRE(m[0] == 'A');
  REQUIRE(m[1] == 'C');
  REQUIRE(m[2] == 'C');
  REQUIRE(m[3] == 'C');
  REQUIRE(m[4] == 'C');
  REQUIRE(m[5] == 'C');
  REQUIRE(m[6] == 'A');
}

TEST_CASE("OverlappingRangeInner") {
  interval_map<int, char> m('A');
  m.assign(1, 6, 'C');
  m.assign(3, 5, 'B');
  REQUIRE(m[0] == 'A');
  REQUIRE(m[1] == 'C');
  REQUIRE(m[2] == 'C');
  REQUIRE(m[3] == 'B');
  REQUIRE(m[4] == 'B');
  REQUIRE(m[5] == 'C');
  REQUIRE(m[6] == 'A');
}

TEST_CASE("OverlappingRangeSmallToLarge") {
  interval_map<int, char> m('A');
  m.assign(1, 5, 'B');
  m.assign(3, 6, 'C');
  REQUIRE(m[0] == 'A');
  REQUIRE(m[1] == 'B');
  REQUIRE(m[2] == 'B');
  REQUIRE(m[3] == 'C');
  REQUIRE(m[4] == 'C');
  REQUIRE(m[5] == 'C');
  REQUIRE(m[6] == 'A');
}

TEST_CASE("OverlappingRangeLargeToSmall") {
  interval_map<int, char> m('A');
  m.assign(3, 6, 'C');
  m.assign(1, 5, 'B');
  REQUIRE(m[0] == 'A');
  REQUIRE(m[1] == 'B');
  REQUIRE(m[2] == 'B');
  REQUIRE(m[3] == 'B');
  REQUIRE(m[4] == 'B');
  REQUIRE(m[5] == 'C');
  REQUIRE(m[6] == 'A');
}

TEST_CASE("ExtendingRangeBegin") {
  interval_map<int, char> m('A');
  m.assign(3, 5, 'B');
  m.assign(1, 4, 'B');
  REQUIRE(m[0] == 'A');
  REQUIRE(m[1] == 'B');
  REQUIRE(m[2] == 'B');
  REQUIRE(m[3] == 'B');
  REQUIRE(m[4] == 'B');
  REQUIRE(m[5] == 'A');
}

TEST_CASE("ExtendingRangeEnd") {
  interval_map<int, char> m('A');
  m.assign(1, 5, 'B');
  m.assign(3, 6, 'B');
  REQUIRE(m[0] == 'A');
  REQUIRE(m[1] == 'B');
  REQUIRE(m[2] == 'B');
  REQUIRE(m[3] == 'B');
  REQUIRE(m[4] == 'B');
  REQUIRE(m[5] == 'B');
  REQUIRE(m[6] == 'A');
}

TEST_CASE("ExtendingRangeBothBeginEnd") {
  interval_map<int, char> m('A');
  m.assign(2, 3, 'B');
  m.assign(1, 5, 'B');
  REQUIRE(m[0] == 'A');
  REQUIRE(m[1] == 'B');
  REQUIRE(m[2] == 'B');
  REQUIRE(m[3] == 'B');
  REQUIRE(m[4] == 'B');
  REQUIRE(m[5] == 'A');
}

TEST_CASE("OverwriteEndValueSafety") {
  interval_map<int, char> m('A');
  m.assign(2, 5, 'B');
  m.assign(5, 8, 'C');
  m.assign(4, 5, 'A');
}

TEST_CASE("ReusingExistingRangeBothBeginEnd") {
  interval_map<int, char> m('A');
  m.assign(1, 5, 'B');
  m.assign(2, 3, 'B');
  REQUIRE(m[0] == 'A');
  REQUIRE(m[1] == 'B');
  REQUIRE(m[2] == 'B');
  REQUIRE(m[3] == 'B');
  REQUIRE(m[4] == 'B');
  REQUIRE(m[5] == 'A');
}

TEST_CASE("ReusingEnd") {
  interval_map<int, char> m('A');
  m.assign(1, 5, 'B');
}
