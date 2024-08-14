#include <catch2/catch_test_macros.hpp>

#include <physbuzz/containers/contigiousmap.hpp>

using ID = std::uint32_t;

struct Box {
    int x;
};

TEST_CASE("Physbuzz::ContiguousMap") {
    Box obj1 = {
        .x = 12,
    };

    Box obj2 = {
        .x = 10,
    };

    SECTION("insert()") {
        Physbuzz::ContiguousMap<ID, Box> map;

        CHECK(map.size() == 0);

        ID id1 = map.insert(1, obj1);
        ID id2 = map.insert(2, obj2);
        ID id3 = map.insert(20, obj2);

        CHECK(map.contains(id1));
        CHECK(map.contains(id2));
        CHECK(map.contains(id3));

        CHECK(id1 == 1);
        CHECK(id2 == 2);
        CHECK(id3 == 20);

        CHECK(map.get(id1).x == obj1.x);
        CHECK(map.get(id2).x == obj2.x);
        CHECK(map.get(id3).x == obj2.x);

        CHECK(map.size() == 3);

        map.insert(20, obj1);

        CHECK(map.contains(id1));
        CHECK(map.contains(id2));
        CHECK(map.contains(id3));

        CHECK(id1 == 1);
        CHECK(id2 == 2);
        CHECK(id3 == 20);

        CHECK(map.get(id1).x == obj1.x);
        CHECK(map.get(id2).x == obj2.x);
        CHECK(map.get(id3).x == obj1.x);

        CHECK(map.size() == 3);
    }

    SECTION("remove()") {
        Physbuzz::ContiguousMap<ID, Box> map;

        ID id1 = map.insert(5, obj1);
        ID id2 = map.insert(4, obj2);
        ID id3 = map.insert(20, obj1);
        ID id4 = map.insert(40, obj2);

        // returns false
        CHECK(!map.erase(42));
        CHECK(!map.contains(42));

        // state check
        CHECK(map.contains(id1));
        CHECK(map.contains(id2));
        CHECK(map.contains(id3));
        CHECK(map.contains(id4));
        CHECK(map.get(id1).x == obj1.x);
        CHECK(map.get(id2).x == obj2.x);
        CHECK(map.get(id3).x == obj1.x);
        CHECK(map.get(id4).x == obj2.x);
        CHECK(map.size() == 4);

        CHECK(map.erase(id2));
        CHECK(!map.erase(id2));

        // state check
        CHECK(map.contains(id1));
        CHECK(!map.contains(id2));
        CHECK(map.contains(id3));
        CHECK(map.contains(id4));
        CHECK(map.get(id1).x == obj1.x);
        CHECK(map.get(id3).x == obj1.x);
        CHECK(map.get(id4).x == obj2.x);
        CHECK(map.size() == 3);

        CHECK(map.erase(id4));
        CHECK(!map.contains(id4));

        // state check
        CHECK(map.contains(id1));
        CHECK(!map.contains(id2));
        CHECK(map.contains(id3));
        CHECK(!map.contains(id4));
        CHECK(map.get(id1).x == obj1.x);
        CHECK(map.get(id3).x == obj1.x);
        CHECK(map.size() == 2);

        // insert after remove
        map.insert(id4, obj1);
        map.insert(id2, obj1);

        // state check
        CHECK(map.contains(id1));
        CHECK(map.contains(id2));
        CHECK(map.contains(id3));
        CHECK(map.contains(id4));
        CHECK(map.get(id1).x == obj1.x);
        CHECK(map.get(id2).x == obj1.x);
        CHECK(map.get(id3).x == obj1.x);
        CHECK(map.get(id4).x == obj1.x);
        CHECK(map.size() == 4);

        CHECK(map.erase(id1));
        CHECK(!map.contains(id1));

        CHECK(map.erase(id2));
        CHECK(!map.contains(id2));

        CHECK(map.erase(id3));
        CHECK(!map.contains(id3));

        CHECK(map.erase(id4));
        CHECK(!map.contains(id4));

        // state check
        CHECK(!map.contains(id1));
        CHECK(!map.contains(id2));
        CHECK(!map.contains(id3));
        CHECK(!map.contains(id4));
        CHECK(map.size() == 0);

        // returns false
        CHECK(!map.erase(id3));
        CHECK(!map.contains(id3));

        // state check
        CHECK(!map.contains(id1));
        CHECK(!map.contains(id2));
        CHECK(!map.contains(id3));
        CHECK(!map.contains(id4));
        CHECK(map.size() == 0);
    }

    SECTION("clear()") {
        Physbuzz::ContiguousMap<ID, Box> map;

        ID id1 = map.insert(1, obj1);
        ID id2 = map.insert(2, obj2);
        ID id3 = map.insert(20, obj1);

        map.clear();

        CHECK(map.size() == 0);
        CHECK(!map.contains(id1));
        CHECK(!map.contains(id2));
        CHECK(!map.contains(id3));
    }

    SECTION("get()") {
        Physbuzz::ContiguousMap<ID, Box> map;

        ID id1 = map.insert(10, obj1);
        ID id2 = map.insert(9, obj2);
        ID id3 = map.insert(20, obj1);
        ID id4 = map.insert(40, obj1);

        Box &obj1 = map.get(id1);
        CHECK(obj1.x == 12);

        Box &obj2 = map.get(id2);
        CHECK(obj2.x == 10);

        Box &obj3 = map.get(id3);
        CHECK(obj3.x == 12);

        Box &obj4 = map.get(id4);
        CHECK(obj4.x == 12);
    }
}
