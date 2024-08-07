#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <physbuzz/events/scene.hpp>
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
        TestComponent test1;

        Physbuzz::ObjectID id1 = scene.createObject();
        Physbuzz::ObjectID id2 = scene.createObject();
        scene.getObject(id1).setComponent(test1);

        const std::vector<Physbuzz::Object> &objects = scene.getObjects();

        CHECK(scene.hasObject(id1));
        CHECK(scene.hasObject(id2));

        Physbuzz::ObjectID id3 = scene.createObject(id1);
        Physbuzz::ObjectID id4 = scene.createObject(id2);
        Physbuzz::ObjectID id5 = scene.createObject(200);

        CHECK(scene.hasObject(id3));
        CHECK(scene.hasObject(id4));

        // check if it overwrote
        CHECK(!scene.getObject(id1).hasComponent<TestComponent>());

        CHECK(id1 == id3);
        CHECK(id2 == id4);
        CHECK(200 == id5);
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
        Physbuzz::ObjectID id3 = scene.createObject();

        // delete from middle
        CHECK(scene.deleteObject(id2));
        CHECK(scene.getObjects().size() == 2);
        CHECK(!scene.deleteObject(id2));
        CHECK(!scene.hasObject(id2));

        // delete from end
        CHECK(scene.deleteObject(id3));
        CHECK(scene.getObjects().size() == 1);
        CHECK(!scene.deleteObject(id3));
        CHECK(!scene.hasObject(id3));

        // delete from start
        CHECK(scene.deleteObject(id1));
        CHECK(scene.getObjects().size() == 0);
        CHECK(!scene.deleteObject(id1));
        CHECK(!scene.hasObject(id1));

        // adding same id should work
        scene.createObject(id3);
        scene.createObject(id2);
        scene.createObject(id1);
        CHECK(scene.hasObject(id3));
        CHECK(scene.hasObject(id2));
        CHECK(scene.hasObject(id1));
    }

    SECTION("getComponents()") {
        Physbuzz::Scene scene;

        CHECK(!scene.existsComponents<TestComponent>());
        CHECK(!scene.existsComponents<TestComponentTheHitSequel>());

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

        CHECK(scene.existsComponents<TestComponent>());
        CHECK(scene.existsComponents<TestComponentTheHitSequel>());
        CHECK(components1.size() == 10);
        CHECK(components2.size() == 5);

        for (int i = 0; i < 10; i++) {
            CHECK(components1[i].x == i);
        }

        for (int i = 0; i < 5; i++) {
            CHECK(components2[i].x == i - 10);
        }
    }

    SECTION("clear()") {
        Physbuzz::Scene scene;

        for (int i = 0; i < 10; i++) {
            Physbuzz::ObjectID id = scene.createObject();

            TestComponent component = {
                .x = i,
            };

            scene.getObject(id).setComponent(component);
        }

        scene.clear();
        CHECK(scene.getObjects().size() == 0);

        for (int i = 0; i < 10; i++) {
            Physbuzz::ObjectID id = scene.createObject();

            TestComponent component = {
                .x = i,
            };

            scene.getObject(id).setComponent(component);
        }

        CHECK(scene.getObjects().size() == 10);
    }

    SECTION("addCallback()") {
        Physbuzz::Scene scene;

        std::vector<int> ids = {1, 2, 3, 4, 10};
        std::vector<int> calledIds = {};

        Physbuzz::EventID idCreate = scene.addCallback<Physbuzz::OnObjectCreateEvent>([&](const Physbuzz::OnObjectCreateEvent &event) {
            REQUIRE(event.scene == &scene);
            calledIds.push_back(event.id);
        });

        Physbuzz::EventID idRemove = scene.addCallback<Physbuzz::OnObjectDeleteEvent>([&](const Physbuzz::OnObjectDeleteEvent &event) {
            REQUIRE(event.scene == &scene);
            calledIds.erase(std::find(calledIds.begin(), calledIds.end(), event.id));
        });

        for (const auto &id : ids) {
            scene.createObject(id);
        }

        for (int i = 0; i < ids.size(); i++) {
            CHECK(ids[i] == calledIds[i]);
        }

        for (const auto &id : ids) {
            scene.deleteObject(id);
        }

        CHECK(calledIds.size() == 0);
    }

    SECTION("removeCallback()") {
        Physbuzz::Scene scene;

        std::vector<int> ids = {1, 2, 3, 4, 10};
        std::vector<int> calledIds = {};

        Physbuzz::EventID idCreate = scene.addCallback<Physbuzz::OnObjectCreateEvent>([&](const Physbuzz::OnObjectCreateEvent &event) {
            CHECK(false);
            calledIds.push_back(event.id);
        });

        scene.removeCallback<Physbuzz::OnObjectCreateEvent>(idCreate);

        for (const auto &id : ids) {
            scene.createObject(id);
        }

        CHECK(calledIds.size() == 0);
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

    SECTION("addCallback()") {
        Physbuzz::Scene scene;

        Physbuzz::ObjectID id1 = scene.createObject();
        Physbuzz::Object object = scene.getObject(id1);

        TestComponent component1 = {
            .x = -12,
        };

        TestComponentTheHitSequel component2 = {
            .x = 10,
        };

        Physbuzz::EventID idCreate1 = object.addCallback<Physbuzz::OnComponentSetEvent<TestComponent>>([&](const Physbuzz::OnComponentSetEvent<TestComponent> &event) {
            REQUIRE(event.object == &object);
            CHECK(event.component.x == -12);
        });

        Physbuzz::EventID idCreate2 = object.addCallback<Physbuzz::OnComponentSetEvent<TestComponentTheHitSequel>>([&](const Physbuzz::OnComponentSetEvent<TestComponentTheHitSequel> &event) {
            REQUIRE(event.object == &object);
            CHECK(event.component.x == 10);
        });

        Physbuzz::EventID idRemove1 = object.addCallback<Physbuzz::OnComponentRemoveEvent<TestComponent>>([&](const Physbuzz::OnComponentRemoveEvent<TestComponent> &event) {
            REQUIRE(event.object == &object);
            CHECK(event.component.x == -12);
        });

        Physbuzz::EventID idRemove2 = object.addCallback<Physbuzz::OnComponentRemoveEvent<TestComponentTheHitSequel>>([&](const Physbuzz::OnComponentRemoveEvent<TestComponentTheHitSequel> &event) {
            REQUIRE(event.object == &object);
            CHECK(event.component.x == 10);
        });

        Physbuzz::EventID idErase = object.addCallback<Physbuzz::OnComponentEraseEvent>([&](const Physbuzz::OnComponentEraseEvent &event) {
            REQUIRE(event.object == &object);
        });

        object.setComponent(component1);
        object.setComponent(component2);

        object.removeComponent<TestComponent>();
        object.removeComponent<TestComponentTheHitSequel>();

        object.eraseComponents();
    }

    SECTION("removeCallback()") {
        Physbuzz::Scene scene;

        Physbuzz::ObjectID id1 = scene.createObject();
        Physbuzz::Object object = scene.getObject(id1);

        TestComponent component1 = {
            .x = -12,
        };

        Physbuzz::EventID idCreate = object.addCallback<Physbuzz::OnComponentSetEvent<TestComponent>>([&](const Physbuzz::OnComponentSetEvent<TestComponent> &event) {
            CHECK(false);
        });
        object.removeCallback<Physbuzz::OnComponentSetEvent<TestComponent>>(idCreate);

        Physbuzz::EventID idRemove = object.addCallback<Physbuzz::OnComponentRemoveEvent<TestComponent>>([&](const Physbuzz::OnComponentRemoveEvent<TestComponent> &event) {
            CHECK(false);
        });
        object.removeCallback<Physbuzz::OnComponentRemoveEvent<TestComponent>>(idRemove);

        object.setComponent(component1);
        object.removeComponent<TestComponent>();
    }
}
