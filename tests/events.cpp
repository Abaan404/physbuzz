#include <catch2/catch_test_macros.hpp>

#include <physbuzz/events.hpp>

struct onMethodCall1 {
    int i;
};

struct onMethodCall2 {
    int j;
};

class EventHandler : public Physbuzz::IEventSubject {
  public:
    void method1(int i) {
        notifyCallbacks<onMethodCall1>({
            .i = i,
        });
    }

    void method2(int j) {
        notifyCallbacks<onMethodCall2>({
            .j = j,
        });
    }
};

TEST_CASE("Physbuzz::IEventSubject") {

    SECTION("addCallback(): One Handle") {
        EventHandler handle;

        std::vector<int> calledIds1;
        std::vector<int> calledIds2;

        handle.addCallback<onMethodCall1>([&](const onMethodCall1 &event) {
            calledIds1.push_back(event.i);
        });

        handle.addCallback<onMethodCall2>([&](const onMethodCall2 &event) {
            calledIds2.push_back(event.j);
        });

        std::vector<int> expected = {1, 2, 3, 4, 5, 100, 1000};

        for (const auto &num : expected) {
            handle.method1(num);
            handle.method2(num);
        }

        for (int i = 0; i < expected.size(); i++) {
            CHECK(calledIds1[i] == expected[i]);
            CHECK(calledIds2[i] == expected[i]);
        }
    }

    SECTION("addCallback(): Two Handles") {
        EventHandler handle1;
        EventHandler handle2;

        std::vector<int> calledIds1;
        std::vector<int> calledIds2;

        handle1.addCallback<onMethodCall1>([&](const onMethodCall1 &event) {
            calledIds1.push_back(event.i);
        });

        handle1.addCallback<onMethodCall2>([&](const onMethodCall2 &event) {
            calledIds1.push_back(event.j + 10);
        });

        handle2.addCallback<onMethodCall1>([&](const onMethodCall1 &event) {
            calledIds2.push_back(event.i + 20);
        });

        handle2.addCallback<onMethodCall2>([&](const onMethodCall2 &event) {
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

        for (int i = 0; i < expected1.size(); i++) {
            CHECK(calledIds1[i] == expected1[i]);
            CHECK(calledIds2[i] == expected2[i]);
        }
    }

    SECTION("removeCallback()") {
        EventHandler handle;

        std::vector<int> calledIds1;
        std::vector<int> calledIds2;

        Physbuzz::EventID id1 = handle.addCallback<onMethodCall1>([&](const onMethodCall1 &event) {
            calledIds1.push_back(event.i);
        });
        handle.removeCallback<onMethodCall1>(id1);

        Physbuzz::EventID id2 = handle.addCallback<onMethodCall2>([&](const onMethodCall2 &event) {
            calledIds2.push_back(event.j);
        });
        handle.removeCallback<onMethodCall2>(id2);

        std::vector<int> expected = {1, 2, 3, 4, 5, 100, 1000};
        for (const auto &num : expected) {
            handle.method1(num);
            handle.method2(num);
        }

        CHECK(calledIds1.size() == 0);
        CHECK(calledIds2.size() == 0);
    }
}
