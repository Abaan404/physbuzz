#include <catch2/catch_test_macros.hpp>

#include <physbuzz/ecs/scene.hpp>
#include <vector>

struct TestComponent1 {
    int x;
};

struct TestComponent2 {
    int x;
};

struct TestComponent3 {
    int z;
};

class TestSystem1 : public Physbuzz::System<TestComponent1, TestComponent3> {
  public:
    void tick(Physbuzz::Scene &scene) {
        for (const auto &id : m_Objects) {
            tickedIds.insert(id);
        }
    }

    std::set<Physbuzz::ObjectID> tickedIds;
};

class TestSystem2 : public Physbuzz::System<TestComponent2, TestComponent3> {
  public:
    void tick(Physbuzz::Scene &scene) {
        for (const auto &id : m_Objects) {
            tickedIds.insert(id);
        }
    }

    std::set<Physbuzz::ObjectID> tickedIds;
};

TEST_CASE("Physbuzz::Scene") {

    SECTION("createObject()") {
        Physbuzz::Scene scene;

        // test obj doesnt exists
        CHECK(!scene.containsObject(12));
        CHECK(!scene.containsObject(15));

        const Physbuzz::ObjectID id1 = scene.createObject(12);
        const Physbuzz::ObjectID id2 = scene.createObject(15);

        // test obj exists
        CHECK(scene.containsObject(id1));
        CHECK(scene.containsObject(id2));
        CHECK(scene.getObjects().size() == 2);

        // obj1 and obj2 should be same
        CHECK(id1 == 12);
        CHECK(id2 == 15);

        // this should do effectively nothing
        const Physbuzz::ObjectID id3 = scene.createObject(id2);

        CHECK(id1 == 12);
        CHECK(id2 == 15);
        CHECK(id3 == id2);
    }

    SECTION("getObjects()") {
        Physbuzz::Scene scene;

        // scene should be empty
        CHECK(scene.getObjects().size() == 0);

        Physbuzz::ObjectID id1 = scene.createObject(42);
        Physbuzz::ObjectID id2 = scene.createObject(99);
        const std::set<Physbuzz::ObjectID> &objects = scene.getObjects();

        // but not anymore
        CHECK(objects.size() == 2);

        // objects must exist (but order is NOT garunteed)
        CHECK(objects.contains(id1));
        CHECK(objects.contains(id2));
    }

    SECTION("containsObject()") {
        Physbuzz::Scene scene;

        // must not exist
        CHECK(!scene.containsObject(15));
        CHECK(!scene.containsObject(12));

        Physbuzz::ObjectID id1 = scene.createObject(15);
        Physbuzz::ObjectID id2 = scene.createObject(12);

        // oh look it exists now!
        CHECK(scene.containsObject(15));
        CHECK(scene.containsObject(12));
    }

    SECTION("deleteObject()") {
        Physbuzz::Scene scene;

        Physbuzz::ObjectID id1 = scene.createObject(1);
        Physbuzz::ObjectID id2 = scene.createObject(2);
        Physbuzz::ObjectID id3 = scene.createObject(99);

        // delete from middle
        CHECK(scene.eraseObject(id2));
        CHECK(scene.getObjects().size() == 2);
        CHECK(!scene.eraseObject(id2));
        CHECK(!scene.containsObject(id2));

        // delete from end
        CHECK(scene.eraseObject(id3));
        CHECK(scene.getObjects().size() == 1);
        CHECK(!scene.eraseObject(id3));
        CHECK(!scene.containsObject(id3));

        // delete from start
        CHECK(scene.eraseObject(id1));
        CHECK(scene.getObjects().size() == 0);
        CHECK(!scene.eraseObject(id1));
        CHECK(!scene.containsObject(id1));

        // adding same id should work
        scene.createObject(id3);
        scene.createObject(id2);
        scene.createObject(id1);
        CHECK(scene.containsObject(id3));
        CHECK(scene.containsObject(id2));
        CHECK(scene.containsObject(id1));
    }

    SECTION("setComponent()") {
        Physbuzz::Scene scene;

        Physbuzz::ObjectID id1 = scene.createObject(5);
        Physbuzz::ObjectID id2 = scene.createObject(4);

        TestComponent1 component1 = {
            .x = 12,
        };

        TestComponent2 component2 = {
            .x = -10,
        };

        // state check
        CHECK(!scene.containsComponent<TestComponent1>(id1));
        CHECK(!scene.containsComponent<TestComponent1>(id2));
        CHECK(!scene.containsComponent<TestComponent2>(id1));
        CHECK(!scene.containsComponent<TestComponent2>(id2));
        CHECK(scene.getComponents<TestComponent1>().size() == 0);
        CHECK(scene.getComponents<TestComponent2>().size() == 0);

        scene.setComponent(id1, component1);
        CHECK(scene.getComponent<TestComponent1>(id1).x == component1.x);

        // state check
        CHECK(scene.containsComponent<TestComponent1>(id1));
        CHECK(!scene.containsComponent<TestComponent1>(id2));
        CHECK(!scene.containsComponent<TestComponent2>(id1));
        CHECK(!scene.containsComponent<TestComponent2>(id2));
        CHECK(scene.getComponents<TestComponent1>().size() == 1);
        CHECK(scene.getComponents<TestComponent2>().size() == 0);

        scene.setComponent(id2, component1);
        CHECK(scene.getComponent<TestComponent1>(id1).x == component1.x);
        CHECK(scene.getComponent<TestComponent1>(id2).x == component1.x);

        // state check
        CHECK(scene.containsComponent<TestComponent1>(id1));
        CHECK(scene.containsComponent<TestComponent1>(id2));
        CHECK(!scene.containsComponent<TestComponent2>(id1));
        CHECK(!scene.containsComponent<TestComponent2>(id2));
        CHECK(scene.getComponents<TestComponent1>().size() == 2);
        CHECK(scene.getComponents<TestComponent2>().size() == 0);

        scene.setComponent(id1, component2);
        CHECK(scene.getComponent<TestComponent1>(id1).x == component1.x);
        CHECK(scene.getComponent<TestComponent1>(id2).x == component1.x);
        CHECK(scene.getComponent<TestComponent2>(id1).x == component2.x);

        // state check
        CHECK(scene.containsComponent<TestComponent1>(id1));
        CHECK(scene.containsComponent<TestComponent1>(id2));
        CHECK(scene.containsComponent<TestComponent2>(id1));
        CHECK(!scene.containsComponent<TestComponent2>(id2));
        CHECK(scene.getComponents<TestComponent1>().size() == 2);
        CHECK(scene.getComponents<TestComponent2>().size() == 1);

        scene.setComponent(id2, component2);
        CHECK(scene.getComponent<TestComponent1>(id1).x == component1.x);
        CHECK(scene.getComponent<TestComponent1>(id2).x == component1.x);
        CHECK(scene.getComponent<TestComponent2>(id1).x == component2.x);
        CHECK(scene.getComponent<TestComponent2>(id2).x == component2.x);

        // state check
        CHECK(scene.containsComponent<TestComponent1>(id1));
        CHECK(scene.containsComponent<TestComponent1>(id2));
        CHECK(scene.containsComponent<TestComponent2>(id1));
        CHECK(scene.containsComponent<TestComponent2>(id2));
        CHECK(scene.getComponents<TestComponent1>().size() == 2);
        CHECK(scene.getComponents<TestComponent2>().size() == 2);

        TestComponent1 component3 = {
            .x = 1984,
        };

        scene.setComponent(id2, component3);
        CHECK(scene.getComponent<TestComponent1>(id1).x == component1.x);
        CHECK(scene.getComponent<TestComponent1>(id2).x == component3.x);
        CHECK(scene.getComponent<TestComponent2>(id1).x == component2.x);
        CHECK(scene.getComponent<TestComponent2>(id2).x == component2.x);

        // state check (unchanged as previous)
        CHECK(scene.containsComponent<TestComponent1>(id1));
        CHECK(scene.containsComponent<TestComponent1>(id2));
        CHECK(scene.containsComponent<TestComponent2>(id1));
        CHECK(scene.containsComponent<TestComponent2>(id2));
        CHECK(scene.getComponents<TestComponent1>().size() == 2);
        CHECK(scene.getComponents<TestComponent2>().size() == 2);

        Physbuzz::ObjectID id3 = scene.createObject(21);

        // set variadic components
        scene.setComponent(id3, component1, component3);
        CHECK(scene.getComponent<TestComponent1>(id1).x == component1.x);
        CHECK(scene.getComponent<TestComponent1>(id2).x == component3.x);
        CHECK(scene.getComponent<TestComponent2>(id1).x == component2.x);
        CHECK(scene.getComponent<TestComponent2>(id2).x == component2.x);
        CHECK(scene.getComponent<TestComponent1>(id3).x == component3.x); // rightmost priority

        // state check
        CHECK(scene.containsComponent<TestComponent1>(id1));
        CHECK(scene.containsComponent<TestComponent1>(id2));
        CHECK(scene.containsComponent<TestComponent1>(id3));
        CHECK(scene.containsComponent<TestComponent2>(id1));
        CHECK(scene.containsComponent<TestComponent2>(id2));
        CHECK(!scene.containsComponent<TestComponent2>(id3));
        CHECK(scene.getComponents<TestComponent1>().size() == 3);
        CHECK(scene.getComponents<TestComponent2>().size() == 2);

        // overwrite variadic
        scene.setComponent(id1, component1, component2, component3);
        CHECK(scene.getComponent<TestComponent1>(id1).x == component3.x); // rightmost priority
        CHECK(scene.getComponent<TestComponent1>(id2).x == component3.x);
        CHECK(scene.getComponent<TestComponent2>(id1).x == component2.x);
        CHECK(scene.getComponent<TestComponent2>(id2).x == component2.x);
        CHECK(scene.getComponent<TestComponent1>(id3).x == component3.x);

        // state check (no change)
        CHECK(scene.containsComponent<TestComponent1>(id1));
        CHECK(scene.containsComponent<TestComponent1>(id2));
        CHECK(scene.containsComponent<TestComponent1>(id3));
        CHECK(scene.containsComponent<TestComponent2>(id1));
        CHECK(scene.containsComponent<TestComponent2>(id2));
        CHECK(!scene.containsComponent<TestComponent2>(id3));
        CHECK(scene.getComponents<TestComponent1>().size() == 3);
        CHECK(scene.getComponents<TestComponent2>().size() == 2);
    }

    SECTION("eraseComponent()") {
        Physbuzz::Scene scene;

        Physbuzz::ObjectID id1 = scene.createObject(99);
        Physbuzz::ObjectID id2 = scene.createObject(11);

        TestComponent1 component1 = {
            .x = 12,
        };

        TestComponent2 component2 = {
            .x = -10,
        };

        TestComponent1 component3 = {
            .x = 1000,
        };

        scene.setComponent(id1, component1);
        scene.setComponent(id2, component1, component2);

        // test is removing an unset component fails
        CHECK(!scene.eraseComponent<TestComponent2>(id1));
        CHECK(scene.getComponents<TestComponent1>().size() == 2);
        CHECK(scene.getComponents<TestComponent2>().size() == 1);

        // remove not empty container
        CHECK(scene.eraseComponent<TestComponent1>(id2));
        CHECK(!scene.eraseComponent<TestComponent1>(id2));
        CHECK(scene.getComponent<TestComponent1>(id1).x == component1.x);
        CHECK(scene.getComponent<TestComponent2>(id2).x == component2.x);

        // state check
        CHECK(scene.containsComponent<TestComponent1>(id1));
        CHECK(!scene.containsComponent<TestComponent1>(id2));
        CHECK(!scene.containsComponent<TestComponent2>(id1));
        CHECK(scene.containsComponent<TestComponent2>(id2));
        CHECK(scene.getComponents<TestComponent1>().size() == 1);
        CHECK(scene.getComponents<TestComponent2>().size() == 1);

        // remove until empty
        CHECK(scene.eraseComponent<TestComponent1>(id1));
        CHECK(!scene.eraseComponent<TestComponent1>(id1));
        CHECK(scene.getComponent<TestComponent2>(id2).x == component2.x);

        // state check
        CHECK(!scene.containsComponent<TestComponent1>(id1));
        CHECK(!scene.containsComponent<TestComponent1>(id2));
        CHECK(!scene.containsComponent<TestComponent2>(id1));
        CHECK(scene.containsComponent<TestComponent2>(id2));
        CHECK(scene.getComponents<TestComponent1>().size() == 0);
        CHECK(scene.getComponents<TestComponent2>().size() == 1);

        // check if setting component at a previously removed location works
        scene.setComponent(id1, component3);
        CHECK(scene.getComponent<TestComponent1>(id1).x == component3.x);
        CHECK(scene.getComponent<TestComponent2>(id2).x == component2.x);

        // state check
        CHECK(scene.containsComponent<TestComponent1>(id1));
        CHECK(!scene.containsComponent<TestComponent1>(id2));
        CHECK(!scene.containsComponent<TestComponent2>(id1));
        CHECK(scene.containsComponent<TestComponent2>(id2));
        CHECK(scene.getComponents<TestComponent1>().size() == 1);
        CHECK(scene.getComponents<TestComponent2>().size() == 1);

        // remove until empty
        CHECK(scene.eraseComponent<TestComponent1>(id1));
        CHECK(!scene.eraseComponent<TestComponent1>(id1));
        CHECK(scene.getComponent<TestComponent2>(id2).x == component2.x);

        CHECK(scene.eraseComponent<TestComponent2>(id2));
        CHECK(!scene.eraseComponent<TestComponent2>(id2));

        // state check
        CHECK(!scene.containsComponent<TestComponent1>(id1));
        CHECK(!scene.containsComponent<TestComponent1>(id2));
        CHECK(!scene.containsComponent<TestComponent2>(id1));
        CHECK(!scene.containsComponent<TestComponent2>(id2));
        CHECK(scene.getComponents<TestComponent1>().size() == 0);
        CHECK(scene.getComponents<TestComponent2>().size() == 0);

        scene.setComponent(id1, component1, component2);
        scene.setComponent(id2, component1);

        // state check
        CHECK(scene.containsComponent<TestComponent1>(id1));
        CHECK(scene.containsComponent<TestComponent1>(id2));
        CHECK(scene.containsComponent<TestComponent2>(id1));
        CHECK(!scene.containsComponent<TestComponent2>(id2));
        CHECK(scene.getComponents<TestComponent1>().size() == 2);
        CHECK(scene.getComponents<TestComponent2>().size() == 1);

        // removes only exclusively these components if exists
        CHECK(scene.eraseComponent<TestComponent1, TestComponent2>(id1));
        CHECK(!scene.eraseComponent<TestComponent1, TestComponent2>(id1));
        CHECK(scene.getComponent<TestComponent1>(id2).x == component1.x);

        // state check
        CHECK(!scene.containsComponent<TestComponent1>(id1));
        CHECK(scene.containsComponent<TestComponent1>(id2));
        CHECK(!scene.containsComponent<TestComponent2>(id1));
        CHECK(!scene.containsComponent<TestComponent2>(id2));
        CHECK(scene.getComponents<TestComponent1>().size() == 1);
        CHECK(scene.getComponents<TestComponent2>().size() == 0);

        // id2 doesnt not contain the exclusive component pair
        CHECK(!scene.eraseComponent<TestComponent2, TestComponent1>(id2));
        CHECK(scene.getComponent<TestComponent1>(id2).x == component1.x);

        // state check (unchanged)
        CHECK(!scene.containsComponent<TestComponent1>(id1));
        CHECK(scene.containsComponent<TestComponent1>(id2));
        CHECK(!scene.containsComponent<TestComponent2>(id1));
        CHECK(!scene.containsComponent<TestComponent2>(id2));
        CHECK(scene.getComponents<TestComponent1>().size() == 1);
        CHECK(scene.getComponents<TestComponent2>().size() == 0);

        // clear object and regenerate one
        scene.createObject(id2);

        CHECK(!scene.containsComponent<TestComponent1>(id1));
        CHECK(!scene.containsComponent<TestComponent1>(id2));
        CHECK(!scene.containsComponent<TestComponent2>(id1));
        CHECK(!scene.containsComponent<TestComponent2>(id2));
        CHECK(scene.getComponents<TestComponent1>().size() == 0);
        CHECK(scene.getComponents<TestComponent2>().size() == 0);
    }

    SECTION("containsComponent()") {
        Physbuzz::Scene scene;

        Physbuzz::ObjectID id1 = scene.createObject(123);
        Physbuzz::ObjectID id2 = scene.createObject(463);

        TestComponent1 component1 = {
            .x = 12,
        };

        TestComponent2 component2 = {
            .x = -10,
        };

        // state check
        CHECK(!scene.containsComponent<TestComponent1>(id1));
        CHECK(!scene.containsComponent<TestComponent1>(id2));
        CHECK(!scene.containsComponent<TestComponent2>(id1));
        CHECK(!scene.containsComponent<TestComponent2>(id2));
        CHECK(!scene.containsComponent<TestComponent1, TestComponent2>(id1));
        CHECK(scene.getComponents<TestComponent1>().size() == 0);
        CHECK(scene.getComponents<TestComponent2>().size() == 0);

        // variadic check
        CHECK(!scene.containsComponent<TestComponent1, TestComponent2>(id1));
        CHECK(!scene.containsComponent<TestComponent1, TestComponent2>(id2));

        scene.setComponent(id1, component1, component2);
        scene.setComponent(id2, component1);

        // state check
        CHECK(scene.containsComponent<TestComponent1>(id1));
        CHECK(scene.containsComponent<TestComponent1>(id2));
        CHECK(scene.containsComponent<TestComponent2>(id1));
        CHECK(!scene.containsComponent<TestComponent2>(id2));
        CHECK(scene.getComponents<TestComponent1>().size() == 2);
        CHECK(scene.getComponents<TestComponent2>().size() == 1);

        // varadic check
        CHECK(scene.containsComponent<TestComponent1, TestComponent2>(id1));
        CHECK(!scene.containsComponent<TestComponent1, TestComponent2>(id2));

        scene.eraseComponent<TestComponent1, TestComponent2>(id1);

        // state check
        CHECK(!scene.containsComponent<TestComponent1>(id1));
        CHECK(scene.containsComponent<TestComponent1>(id2));
        CHECK(!scene.containsComponent<TestComponent2>(id1));
        CHECK(!scene.containsComponent<TestComponent2>(id2));
        CHECK(scene.getComponents<TestComponent1>().size() == 1);
        CHECK(scene.getComponents<TestComponent2>().size() == 0);

        // varadic check
        CHECK(!scene.containsComponent<TestComponent1, TestComponent2>(id1));
        CHECK(!scene.containsComponent<TestComponent1, TestComponent2>(id2));

        scene.setComponent(id1, component1, component2);
        scene.setComponent(id2, component2);

        // state check
        CHECK(scene.containsComponent<TestComponent1>(id1));
        CHECK(scene.containsComponent<TestComponent1>(id2));
        CHECK(scene.containsComponent<TestComponent2>(id1));
        CHECK(scene.containsComponent<TestComponent2>(id2));
        CHECK(scene.getComponents<TestComponent1>().size() == 2);
        CHECK(scene.getComponents<TestComponent2>().size() == 2);

        // varadic check
        CHECK(scene.containsComponent<TestComponent1, TestComponent2>(id1));
        CHECK(scene.containsComponent<TestComponent1, TestComponent2>(id2));
    }

    SECTION("emplaceSystem()") {
        Physbuzz::Scene scene;

        CHECK(!scene.containsSystem<TestSystem1>());
        CHECK(!scene.containsSystem<TestSystem2>());

        scene.emplaceSystem<TestSystem1>();
        scene.emplaceSystem<TestSystem2>();

        CHECK(scene.containsSystem<TestSystem1>());
        CHECK(scene.containsSystem<TestSystem2>());
    }

    SECTION("emplaceSystem()") {
        Physbuzz::Scene scene;

        CHECK(!scene.containsSystem<TestSystem1>());
        CHECK(!scene.containsSystem<TestSystem2>());

        scene.emplaceSystem<TestSystem1>();
        scene.emplaceSystem<TestSystem2>();

        CHECK(scene.containsSystem<TestSystem1>());
        CHECK(scene.containsSystem<TestSystem2>());
    }

    SECTION("removeSystem()") {
        Physbuzz::Scene scene;

        scene.emplaceSystem<TestSystem1>();
        scene.emplaceSystem<TestSystem2>();

        CHECK(scene.containsSystem<TestSystem1>());
        CHECK(scene.containsSystem<TestSystem2>());

        scene.eraseSystem<TestSystem1>();
        scene.eraseSystem<TestSystem2>();

        CHECK(!scene.containsSystem<TestSystem1>());
        CHECK(!scene.containsSystem<TestSystem2>());
    }

    SECTION("tickSystem()") {
        Physbuzz::Scene scene;
        scene.emplaceSystem<TestSystem1>();
        scene.emplaceSystem<TestSystem2>();

        Physbuzz::ObjectID id1 = scene.createObject(1);
        Physbuzz::ObjectID id2 = scene.createObject(2);
        Physbuzz::ObjectID id3 = scene.createObject(4);
        Physbuzz::ObjectID id4 = scene.createObject(3);
        Physbuzz::ObjectID id5 = scene.createObject(5);

        TestComponent1 component1 = {
            .x = 12,
        };

        TestComponent2 component2 = {
            .x = 24,
        };

        TestComponent3 component3 = {
            .z = 99,
        };

        // ticked null
        scene.setComponent(id1, component3);

        // ticked 1
        scene.setComponent(id2, component1);
        scene.setComponent(id2, component3);

        // ticked 2
        scene.setComponent(id3, component2);
        scene.setComponent(id3, component3);

        // ticked 1 and 2
        scene.setComponent(id4, component1);
        scene.setComponent(id4, component2);
        scene.setComponent(id4, component3);

        // ticked 1
        scene.setComponent(id5, component1);
        scene.setComponent(id5, component3);

        scene.tickSystem<TestSystem1>();
        scene.tickSystem<TestSystem2>();

        std::shared_ptr<TestSystem1> system1 = nullptr;
        std::shared_ptr<TestSystem2> system2 = nullptr;

        system1 = scene.getSystem<TestSystem1>();
        system2 = scene.getSystem<TestSystem2>();

        CHECK(system1->tickedIds.size() == 3);
        CHECK(!system1->tickedIds.contains(id1));
        CHECK(system1->tickedIds.contains(id2));
        CHECK(!system1->tickedIds.contains(id3));
        CHECK(system1->tickedIds.contains(id4));
        CHECK(system1->tickedIds.contains(id5));

        CHECK(system2->tickedIds.size() == 2);
        CHECK(!system2->tickedIds.contains(id1));
        CHECK(!system2->tickedIds.contains(id2));
        CHECK(system2->tickedIds.contains(id3));
        CHECK(system2->tickedIds.contains(id4));
        CHECK(!system2->tickedIds.contains(id5));

        system1->tickedIds.clear();
        system2->tickedIds.clear();

        scene.eraseComponent<TestComponent3>(id4);
        // scene.deleteObject(id2);

        scene.tickSystem<TestSystem1>();
        scene.tickSystem<TestSystem2>();

        system1 = scene.getSystem<TestSystem1>();
        system2 = scene.getSystem<TestSystem2>();

        CHECK(system1->tickedIds.size() == 2);
        CHECK(!system1->tickedIds.contains(id1));
        CHECK(system1->tickedIds.contains(id2));
        CHECK(!system1->tickedIds.contains(id3));
        CHECK(!system1->tickedIds.contains(id4));
        CHECK(system1->tickedIds.contains(id5));

        CHECK(system2->tickedIds.size() == 1);
        CHECK(!system2->tickedIds.contains(id1));
        CHECK(!system2->tickedIds.contains(id2));
        CHECK(system2->tickedIds.contains(id3));
        CHECK(!system2->tickedIds.contains(id4));
        CHECK(!system2->tickedIds.contains(id5));

        scene.eraseSystem<TestSystem1>();
        scene.eraseSystem<TestSystem2>();
    }

    SECTION("IEventSubject()") {
        Physbuzz::Scene scene;

        static int ticks = 0;

        // scene.addCallback(std::function<void (const T &)> callback)

        // scene.addCallback
    }
}
