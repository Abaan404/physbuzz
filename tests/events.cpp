#include <catch2/catch_test_macros.hpp>

#include <physbuzz/events/handler.hpp>

struct OnMethodCall1 {
    int i;
};

struct OnMethodCall2 {
    int j;
};

class EventHandler : public Physbuzz::EventSubject {
  public:
    void method1(int i) {
        notifyCallbacks<OnMethodCall1>({
            .i = i,
        });
    }

    void method2(int j) {
        notifyCallbacks<OnMethodCall2>({
            .j = j,
        });
    }
};

TEST_CASE("Physbuzz::IEventSubject") {

    SECTION("addCallback()") {
        EventHandler handle1;
        EventHandler handle2;

        std::vector<int> calledIds1;
        std::vector<int> calledIds2;

        handle1.addCallback<OnMethodCall1>([&](const OnMethodCall1 &event) {
            calledIds1.push_back(event.i);
        });

        handle1.addCallback<OnMethodCall2>([&](const OnMethodCall2 &event) {
            calledIds1.push_back(event.j + 10);
        });

        handle2.addCallback<OnMethodCall1>([&](const OnMethodCall1 &event) {
            calledIds2.push_back(event.i + 20);
        });

        handle2.addCallback<OnMethodCall2>([&](const OnMethodCall2 &event) {
            calledIds2.push_back(event.j + 30);
        });

        std::vector<int> input = {1, 2, 3, 4, 5, 100, 1000};

        std::vector<int> expected1 = {1, 11, 2, 12, 3, 13, 4, 14, 5, 15, 100, 110, 1000, 1010};
        std::vector<int> expected2 = {21, 31, 22, 32, 23, 33, 24, 34, 25, 35, 120, 130, 1020, 1030};

        for (const auto &num : input) {
            handle1.method1(num);
            handle1.method2(num);

            handle2.method1(num);
            handle2.method2(num);
        }

        REQUIRE(expected1.size() == calledIds1.size());
        REQUIRE(expected2.size() == calledIds2.size());

        for (int i = 0; i < expected1.size(); i++) {
            CHECK(calledIds1[i] == expected1[i]);
            CHECK(calledIds2[i] == expected2[i]);
        }
    }

    SECTION("removeCallback()") {
        EventHandler handle;

        std::vector<int> calledIds1;
        std::vector<int> calledIds2;

        Physbuzz::EventID id1 = handle.addCallback<OnMethodCall1>([&](const OnMethodCall1 &event) {
            calledIds1.push_back(event.i);
        });

        Physbuzz::EventID id2 = handle.addCallback<OnMethodCall2>([&](const OnMethodCall2 &event) {
            calledIds2.push_back(event.j);
        });
        handle.eraseCallback<OnMethodCall2>(id2);

        std::vector<int> expected = {1, 2, 3, 4, 5, 100, 1000};
        for (const auto &num : expected) {
            handle.method1(num);
            handle.method2(num);
        }

        REQUIRE(calledIds1.size() == expected.size());
        REQUIRE(calledIds2.size() == 0);

        for (int i = 0; i < expected.size(); i++) {
            CHECK(calledIds1[i] == expected[i]);
        }
    }

    SECTION("clearCallbacks()") {
        EventHandler handle;

        std::vector<int> calledIds1;
        std::vector<int> calledIds2;

        Physbuzz::EventID id1 = handle.addCallback<OnMethodCall1>([&](const OnMethodCall1 &event) {
            calledIds1.push_back(event.i);
        });

        Physbuzz::EventID id2 = handle.addCallback<OnMethodCall2>([&](const OnMethodCall2 &event) {
            calledIds2.push_back(event.j);
        });

        handle.clearCallbacks();

        std::vector<int> expected = {1, 2, 3, 4, 5, 100, 1000};
        for (const auto &num : expected) {
            handle.method1(num);
            handle.method2(num);
        }

        REQUIRE(calledIds1.size() == 0);
        REQUIRE(calledIds2.size() == 0);
    }
}
