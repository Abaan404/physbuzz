#include <physbuzz/resources/registry.hpp>
#include <physbuzz/resources/table.hpp>

#include <catch2/catch_test_macros.hpp>

struct ResourceA {
    std::function<bool()> build = []() {
        return true;
    };

    std::function<bool()> destroy = []() {
        return true;
    };

    std::uint32_t data = 0;

    bool alive = false;
};

struct ResourceB {
    std::uint32_t data = 1;
};

template <>
struct Physbuzz::IsResource<ResourceA> : std::true_type {};

template <>
struct Physbuzz::IsResource<ResourceB> : std::true_type {};

TEST_CASE("Physbuzz::ResourceRegistry") {
    SECTION("insert()") {
        // should be empty exist
        CHECK(!Physbuzz::ResourceRegistry<ResourceA>::contains("res_1"));
        CHECK(!Physbuzz::ResourceRegistry<ResourceA>::contains("res_2"));
        CHECK(!Physbuzz::ResourceRegistry<ResourceB>::contains("res_1"));
        CHECK(!Physbuzz::ResourceRegistry<ResourceB>::contains("res_2"));

        // only 1a exists
        auto res1a = *Physbuzz::ResourceRegistry<ResourceA>::insert("res_1", {});
        CHECK(!Physbuzz::ResourceRegistry<ResourceA>::insert("res_1", {}));

        CHECK(res1a != Physbuzz::ResourceEmptyID);
        CHECK(Physbuzz::ResourceRegistry<ResourceA>::contains("res_1"));
        CHECK(!Physbuzz::ResourceRegistry<ResourceA>::contains("res_2"));

        // only 2a exists
        auto res2a = *Physbuzz::ResourceRegistry<ResourceA>::insert("res_2", {});
        CHECK(!Physbuzz::ResourceRegistry<ResourceA>::insert("res_2", {}));

        CHECK(res2a != Physbuzz::ResourceEmptyID);
        CHECK(Physbuzz::ResourceRegistry<ResourceA>::contains("res_1"));
        CHECK(Physbuzz::ResourceRegistry<ResourceA>::contains("res_2"));

        // only 1b exists
        auto res1b = *Physbuzz::ResourceRegistry<ResourceB>::insert("res_1", {});
        CHECK(!Physbuzz::ResourceRegistry<ResourceB>::insert("res_1", {}));

        CHECK(res1b != Physbuzz::ResourceEmptyID);
        CHECK(Physbuzz::ResourceRegistry<ResourceB>::contains("res_1"));
        CHECK(!Physbuzz::ResourceRegistry<ResourceB>::contains("res_2"));

        // only 2b exists
        auto res2b = *Physbuzz::ResourceRegistry<ResourceB>::insert("res_2", {});
        CHECK(!Physbuzz::ResourceRegistry<ResourceB>::insert("res_2", {}));

        CHECK(res2b != Physbuzz::ResourceEmptyID);
        CHECK(Physbuzz::ResourceRegistry<ResourceB>::contains("res_1"));
        CHECK(Physbuzz::ResourceRegistry<ResourceB>::contains("res_2"));
    }

    SECTION("get()") {
        auto res1a = *Physbuzz::ResourceRegistry<ResourceA>::insert(
            "res_1",
            {
                .data = 20,
            });

        auto res2a = *Physbuzz::ResourceRegistry<ResourceA>::insert(
            "res_2",
            {
                .data = 40,
            });

        auto res1a_1 = *Physbuzz::ResourceRegistry<ResourceA>::get("res_1");
        auto res2a_1 = *Physbuzz::ResourceRegistry<ResourceA>::get("res_2");

        CHECK(res1a.getId() == res1a_1.getId());
        CHECK(res2a.getId() == res2a_1.getId());

        CHECK(res1a.getId() != res2a.getId());

        CHECK(res1a.get().data == 20);
        CHECK(res2a.get().data == 40);

        CHECK(res1a.getName() == "res_1");
        CHECK(res2a.getName() == "res_2");
    }

    SECTION("getRefCounts()") {
        // starts empty
        CHECK(Physbuzz::ResourceRegistry<ResourceA>::getRefCount("res_1") == 0);

        // via insert
        auto res1a = *Physbuzz::ResourceRegistry<ResourceA>::insert("res_1", {});
        CHECK(Physbuzz::ResourceRegistry<ResourceA>::getRefCount("res_1") == 1);

        auto res2a = *Physbuzz::ResourceRegistry<ResourceA>::insert("res_2", {});
        CHECK(Physbuzz::ResourceRegistry<ResourceA>::getRefCount("res_2") == 1);

        // via get
        auto res_get = *Physbuzz::ResourceRegistry<ResourceA>::get("res_1");
        CHECK(Physbuzz::ResourceRegistry<ResourceA>::getRefCount("res_1") == 2);

        // via ctor
        Physbuzz::Resource<ResourceA> res_ctor = {res1a.getId()};
        CHECK(Physbuzz::ResourceRegistry<ResourceA>::getRefCount("res_1") == 3);

        // via copy ctor
        Physbuzz::Resource<ResourceA> res_copy = {res1a};
        CHECK(Physbuzz::ResourceRegistry<ResourceA>::getRefCount("res_1") == 4);

        // via move ctor
        Physbuzz::Resource<ResourceA> res_tmp1 = {res1a};
        CHECK(Physbuzz::ResourceRegistry<ResourceA>::getRefCount("res_1") == 5);

        Physbuzz::Resource<ResourceA> res_move = std::move(res_tmp1);
        CHECK(Physbuzz::ResourceRegistry<ResourceA>::getRefCount("res_1") == 5);
        CHECK(res_tmp1.getId() == Physbuzz::ResourceEmptyID);

        // via copy operator
        res_copy = res2a;
        CHECK(Physbuzz::ResourceRegistry<ResourceA>::getRefCount("res_1") == 4);
        CHECK(Physbuzz::ResourceRegistry<ResourceA>::getRefCount("res_2") == 2);

        // via move operator
        Physbuzz::Resource<ResourceA> res_tmp2 = {res2a};
        CHECK(Physbuzz::ResourceRegistry<ResourceA>::getRefCount("res_2") == 3);

        res_move = std::move(res_tmp2);
        CHECK(Physbuzz::ResourceRegistry<ResourceA>::getRefCount("res_1") == 3);
        CHECK(Physbuzz::ResourceRegistry<ResourceA>::getRefCount("res_2") == 3);
        CHECK(res_tmp2.getId() == Physbuzz::ResourceEmptyID);

        {
            Physbuzz::Resource<ResourceA> res_dtor = {res1a};
            CHECK(Physbuzz::ResourceRegistry<ResourceA>::getRefCount("res_1") == 4);
        }

        CHECK(Physbuzz::ResourceRegistry<ResourceA>::getRefCount("res_1") == 3);
    }

    SECTION("erase") {
        int counter = 0;

        ResourceA resa = {
            .build = [&]() {
                counter++;
                return true;
            },
            .destroy = [&]() {
                counter--;
                return true;
            },
        };

        // call build() implicitly
        auto resa1 = resa;
        auto res1a = *Physbuzz::ResourceRegistry<ResourceA>::insert("res_1", std::move(resa1));
        CHECK(counter == 1);

        auto resa2 = resa;
        auto res2a = *Physbuzz::ResourceRegistry<ResourceA>::insert("res_2", std::move(resa2));
        CHECK(counter == 2);

        // ensure it exists
        CHECK(Physbuzz::ResourceRegistry<ResourceA>::contains("res_1"));
        CHECK(Physbuzz::ResourceRegistry<ResourceA>::contains("res_2"));

        // modify ref counts
        auto res1a_1 = res1a;
        auto res2a_2 = res2a;
        CHECK(counter == 2);

        // drop ref count but not enough to destroy
        res1a_1 = {};
        res2a_2 = {};
        CHECK(counter == 2);

        // drop refs to 0, call destroy() implicitly
        res1a = {};
        CHECK(counter == 1);

        res2a = {};
        CHECK(counter == 0);
    }

    SECTION("Events") {
        std::unordered_set<Physbuzz::ResourceID> data;

        // create instances in callbacks
        Physbuzz::EventID add = Physbuzz::ResourceRegistry<ResourceA>::Events.addCallback<Physbuzz::OnResourceBuild>([&](const auto &event) {
            Physbuzz::Resource<ResourceA> res = event.id;
            data.insert(res->data);
        });

        Physbuzz::EventID remove = Physbuzz::ResourceRegistry<ResourceA>::Events.addCallback<Physbuzz::OnResourceDestroy>([&](const auto &event) {
            Physbuzz::Resource<ResourceA> res = event.id;
            data.erase(res->data);
        });

        auto res1a = *Physbuzz::ResourceRegistry<ResourceA>::insert(
            "res_1",
            {.data = 1});

        auto res2a = *Physbuzz::ResourceRegistry<ResourceA>::insert(
            "res_2",
            {.data = 2});

        auto res3a = *Physbuzz::ResourceRegistry<ResourceA>::insert(
            "res_3",
            {.data = 3});

        CHECK(data.contains(1));
        CHECK(data.contains(2));
        CHECK(data.contains(3));

        res1a = {};
        res3a = {};
        CHECK(!data.contains(1));
        CHECK(data.contains(2));
        CHECK(!data.contains(3));

        // make sure references were freed after callbacks
        CHECK(!Physbuzz::ResourceRegistry<ResourceA>::contains("res_1"));
        CHECK(Physbuzz::ResourceRegistry<ResourceA>::contains("res_2"));
        CHECK(!Physbuzz::ResourceRegistry<ResourceA>::contains("res_3"));

        Physbuzz::ResourceRegistry<ResourceA>::Events.eraseCallback<Physbuzz::OnResourceBuild>(add);
        Physbuzz::ResourceRegistry<ResourceA>::Events.eraseCallback<Physbuzz::OnResourceDestroy>(remove);
    }
}

TEST_CASE("Physbuzz::ResourceTable") {
    SECTION("add()/remove()") {
        Physbuzz::ResourceTable<ResourceA> table;

        auto res1 = *Physbuzz::ResourceRegistry<ResourceA>::insert("res_1", {});
        auto res2 = *Physbuzz::ResourceRegistry<ResourceA>::insert("res_2", {});
        auto res3 = *Physbuzz::ResourceRegistry<ResourceA>::insert("res_3", {});

        // shouldnt exist
        CHECK(!table.contains(res1));
        CHECK(!table.contains(res2));
        CHECK(!table.contains(res2));
        CHECK(table.size() == 0);

        // add
        CHECK(table.add(res1));
        CHECK(table.add(res2));

        // only 1 and 2 exist
        CHECK(table.contains(res1));
        CHECK(table.contains(res2));
        CHECK(!table.contains(res3));
        CHECK(table.size() == 2);

        // add again
        CHECK(!table.add(res1));
        CHECK(!table.add(res2));
        CHECK(table.add(res3));

        // all exist
        CHECK(table.contains(res1));
        CHECK(table.contains(res2));
        CHECK(table.contains(res3));
        CHECK(table.size() == 3);

        // remove
        CHECK(table.remove(res1));
        CHECK(table.remove(res3));

        // 1 and 3 is removed
        CHECK(!table.contains(res1));
        CHECK(table.contains(res2));
        CHECK(!table.contains(res3));
        CHECK(table.size() == 1);

        // remove again
        CHECK(!table.remove(res1));
        CHECK(table.remove(res2));
        CHECK(!table.remove(res3));

        CHECK(!table.contains(res1));
        CHECK(!table.contains(res2));
        CHECK(!table.contains(res3));
        CHECK(table.size() == 0);
    }

    SECTION("query()") {
        Physbuzz::ResourceTable<ResourceA> table;

        auto res1 = *Physbuzz::ResourceRegistry<ResourceA>::insert("res_1", {});
        auto res2 = *Physbuzz::ResourceRegistry<ResourceA>::insert("res_2", {});
        auto res3 = *Physbuzz::ResourceRegistry<ResourceA>::insert("res_3", {});

        // no queries
        CHECK(table.query(res1) == -1u);
        CHECK(table.query(res2) == -1u);
        CHECK(table.query(res3) == -1u);
        CHECK(table.max_size() == 0);

        // counted
        table.add(res1);
        table.add(res3);

        CHECK(table.query(res1) == 0);
        CHECK(table.query(res2) == -1u);
        CHECK(table.query(res3) == 1);
        CHECK(table.max_size() == 2);

        // removed from back
        table.remove(res1);

        CHECK(table.query(res1) == -1u);
        CHECK(table.query(res2) == -1u);
        CHECK(table.query(res3) == 1);
        CHECK(table.max_size() == 2);

        table.add(res2);
        table.add(res1);

        CHECK(table.query(res1) == 2);
        CHECK(table.query(res2) == 0);
        CHECK(table.query(res3) == 1);
        CHECK(table.max_size() == 3);

        // removed from middle
        table.remove(res3);

        CHECK(table.query(res1) == 2);
        CHECK(table.query(res2) == 0);
        CHECK(table.query(res3) == -1u);
        CHECK(table.max_size() == 3);

        table.add(res3);

        CHECK(table.query(res1) == 2);
        CHECK(table.query(res2) == 0);
        CHECK(table.query(res3) == 1);
        CHECK(table.max_size() == 3);

        // removed from end
        table.remove(res1);

        CHECK(table.query(res1) == -1u);
        CHECK(table.query(res2) == 0);
        CHECK(table.query(res3) == 1);
        CHECK(table.max_size() == 2);
    }
}
