#include <catch2/catch_test_macros.hpp>

#include <physbuzz/scene.hpp>

struct TestComponent {
    int x;
};

struct TestComponentTheHitSequel {
    int x;
};

TEST_CASE("Physbuzz::Scene") {

    SECTION("createObject()") {
        Physbuzz::Scene scene;

        Physbuzz::ObjectID id1 = scene.createObject();
        Physbuzz::ObjectID id2 = scene.createObject();

        const std::vector<Physbuzz::Object> &objects = scene.getObjects();

        CHECK(objects[0].getId() == id1);
        CHECK(objects[1].getId() == id2);
    }

    SECTION("getObjects()") {
        Physbuzz::Scene scene;

        Physbuzz::ObjectID object1 = scene.createObject();
        Physbuzz::ObjectID object2 = scene.createObject();

        const std::vector<Physbuzz::Object> &objects = scene.getObjects();

        CHECK(objects.size() == 2);
    }

    SECTION("hasObject()") {
        Physbuzz::Scene scene;

        Physbuzz::ObjectID id1 = scene.createObject();
        Physbuzz::ObjectID id2 = scene.createObject();

        CHECK(scene.hasObject(id1));
        CHECK(scene.hasObject(id2));
    }

    SECTION("deleteObject()") {
        Physbuzz::Scene scene;

        Physbuzz::ObjectID id1 = scene.createObject();
        Physbuzz::ObjectID id2 = scene.createObject();

        const std::vector<Physbuzz::Object> &objects = scene.getObjects();

        scene.deleteObject(id2);

        CHECK(scene.hasObject(id1));
        CHECK(!scene.hasObject(id2));
    }

    SECTION("getComponents") {
        Physbuzz::Scene scene;

        for (int i = 0; i < 10; i++) {
            Physbuzz::ObjectID id = scene.createObject();

            TestComponent component = {
                .x = i,
            };

            scene.getObject(id).setComponent(component);
        }

        for (int i = 10; i > 5; i--) {
            Physbuzz::ObjectID id = scene.createObject();

            TestComponentTheHitSequel component = {
                .x = -i,
            };

            scene.getObject(id).setComponent(component);
        }

        std::vector<TestComponent> &components1 = scene.getComponents<TestComponent>();
        std::vector<TestComponentTheHitSequel> &components2 = scene.getComponents<TestComponentTheHitSequel>();

        CHECK(components1.size() == 10);
        CHECK(components2.size() == 5);

        for (int i = 0; i < 10; i++) {
            CHECK(components1[i].x == i);
        }

        for (int i = 0; i < 5; i++) {
            CHECK(components2[i].x == i - 10);
        }
    }
}

TEST_CASE("Physbuzz::Object") {

    SECTION("getId()") {
        Physbuzz::Scene scene;

        Physbuzz::ObjectID id1 = scene.createObject();
        Physbuzz::Object object = scene.getObject(id1);

        // check if id matches with scene
        CHECK(object.getId() == id1);
    }

    SECTION("setComponent()") {
        Physbuzz::Scene scene;

        Physbuzz::ObjectID id1 = scene.createObject();
        Physbuzz::Object object = scene.getObject(id1);

        {
            TestComponent component = {
                .x = 12,
            };

            object.setComponent(component);

            // test if exists
            CHECK(object.hasComponent<TestComponent>());
            CHECK(scene.getComponents<TestComponent>().size() == 1);

            // test if it matches the expected value
            CHECK(object.getComponent<TestComponent>().x == 12);
        }

        {
            TestComponent component = {
                .x = 10,
            };

            object.setComponent(component);

            // test if it exists
            CHECK(object.hasComponent<TestComponent>());
            CHECK(scene.getComponents<TestComponent>().size() == 1);

            // test if it overwrote the previous value
            CHECK(object.getComponent<TestComponent>().x == 10);
        }

        {
            TestComponentTheHitSequel component = {
                .x = -10,
            };

            object.setComponent(component);

            // test if it exists
            CHECK(object.hasComponent<TestComponent>());
            CHECK(object.hasComponent<TestComponentTheHitSequel>());
            CHECK(scene.getComponents<TestComponent>().size() == 1);
            CHECK(scene.getComponents<TestComponentTheHitSequel>().size() == 1);

            // test if it has the right values
            CHECK(object.getComponent<TestComponent>().x == 10);
            CHECK(object.getComponent<TestComponentTheHitSequel>().x == -10);
        }
    }

    SECTION("removeComponent()") {
        Physbuzz::Scene scene;

        Physbuzz::ObjectID id1 = scene.createObject();
        Physbuzz::Object object = scene.getObject(id1);

        TestComponent component = {
            .x = 12,
        };

        object.setComponent(component);

        // test if removal is successful
        CHECK(object.removeComponent<TestComponent>());

        // test if removal actually removed
        CHECK(!object.hasComponent<TestComponent>());
        CHECK(scene.getComponents<TestComponent>().size() == 0);

        // test if removing again fails
        CHECK(!object.removeComponent<TestComponent>());
    }

    SECTION("eraseComponents()") {
        Physbuzz::Scene scene;

        Physbuzz::ObjectID id1 = scene.createObject();
        Physbuzz::Object object = scene.getObject(id1);

        object.eraseComponents();

        TestComponent component1 = {
            .x = -12,
        };

        TestComponentTheHitSequel component2 = {
            .x = 10,
        };

        object.setComponent(component1);
        object.setComponent(component2);

        object.eraseComponents();

        // check if it doesnt have the components
        CHECK(!object.hasComponent<TestComponent>());
        CHECK(!object.hasComponent<TestComponentTheHitSequel>());

        // and the scene no longer retains the components
        CHECK(scene.getComponents<TestComponent>().size() == 0);
        CHECK(scene.getComponents<TestComponentTheHitSequel>().size() == 0);
    }
}
