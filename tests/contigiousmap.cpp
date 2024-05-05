#include <catch2/catch_test_macros.hpp>

#include <physbuzz/containers/contigiousmap.hpp>

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
        Physbuzz::ContiguousMap<ID, Object> map;

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
        Physbuzz::ContiguousMap<ID, Object> map;

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
        Physbuzz::ContiguousMap<ID, Object> map;

        ID id1 = map.insert(obj1);
        ID id2 = map.insert(obj2);
        ID id3 = map.insert(obj1, 20);

        CHECK(map.size() == 3);

        map.clear();

        CHECK(map.size() == 0);
    }

    SECTION("remove()") {
        Physbuzz::ContiguousMap<ID, Object> map;

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
        Physbuzz::ContiguousMap<ID, Object> map;

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

    SECTION("sort()") {
        Physbuzz::ContiguousMap<ID, Object> map;

        ID id1 = map.insert(Object{.x = 50});
        ID id2 = map.insert(Object{.x = 30});
        ID id3 = map.insert(Object{.x = 20});
        ID id4 = map.insert(Object{.x = 10});

        map.sort([](Object &obj1, Object &obj2) {
            return obj1.x < obj2.x;
        });

        std::vector<Object> &array = map.getArray();

        // check if id is still valid
        CHECK(map.get(id1).x == 50);
        CHECK(map.get(id2).x == 30);
        CHECK(map.get(id3).x == 20);
        CHECK(map.get(id4).x == 10);

        // check if vector has been sorted
        CHECK(array[0].x == 10);
        CHECK(array[1].x == 20);
        CHECK(array[2].x == 30);
        CHECK(array[3].x == 50);

        // why not
        CHECK(map.size() == 4);
    }
}
