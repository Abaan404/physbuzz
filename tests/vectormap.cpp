#include <catch2/catch_test_macros.hpp>

#include <physbuzz/containers/vectormap.hpp>

using ID = std::uint32_t;

struct Object {
    int x;
};

TEST_CASE("Physbuzz::VectorMap") {
    Object obj1 = {
        .x = 12,
    };

    Object obj2 = {
        .x = 10,
    };

    SECTION("insert()") {
        Physbuzz::VectorMap<ID, Object> map;

        ID id1 = map.insert(obj1);
        ID id2 = map.insert(obj2);
        ID id3 = map.insert(obj1, 20);

        CHECK(map.contains(id1));
        CHECK(map.contains(id2));
        CHECK(map.contains(id3));
        CHECK(id3 == 20);
        CHECK(map.size() == 3);
    }

    SECTION("insert() overwrite") {
        Physbuzz::VectorMap<ID, Object> map;

        ID id1 = map.insert(obj1);
        ID id2 = map.insert(obj2, 20);
        map.insert(obj1, 20);

        CHECK(map.contains(id1));
        CHECK(map.contains(id2));

        Object obj = map.get(20);

        CHECK(obj.x == obj1.x);

        CHECK(map.size() == 2);
    }

    SECTION("clear()") {
        Physbuzz::VectorMap<ID, Object> map;

        ID id1 = map.insert(obj1);
        ID id2 = map.insert(obj2);
        ID id3 = map.insert(obj1, 20);

        CHECK(map.size() == 3);

        map.clear();

        CHECK(map.size() == 0);
    }

    SECTION("remove()") {
        Physbuzz::VectorMap<ID, Object> map;

        ID id1 = map.insert(obj1);
        ID id2 = map.insert(obj2);
        ID id3 = map.insert(obj1, 20);
        ID id4 = map.insert(obj1, 40);

        // returns false
        CHECK(!map.remove(42));
        CHECK(!map.contains(42));

        CHECK(map.remove(id2));
        CHECK(!map.contains(id2));

        CHECK(map.remove(id1));
        CHECK(!map.contains(id1));

        CHECK(map.remove(id4));
        CHECK(!map.contains(id4));

        CHECK(map.remove(id3));
        CHECK(!map.contains(id4));

        // returns false
        CHECK(!map.remove(id3));
        CHECK(!map.contains(id3));

        CHECK(map.size() == 0);
    }

    SECTION("get()") {
        Physbuzz::VectorMap<ID, Object> map;

        ID id1 = map.insert(obj1);
        ID id2 = map.insert(obj2);
        ID id3 = map.insert(obj1, 20);
        ID id4 = map.insert(obj1, 40);

        Object &obj1 = map.get(id1);
        CHECK(obj1.x == 12);

        Object &obj2 = map.get(id2);
        CHECK(obj2.x == 10);

        Object &obj3 = map.get(id3);
        CHECK(obj3.x == 12);

        Object &obj4 = map.get(id4);
        CHECK(obj4.x == 12);
    }
}
