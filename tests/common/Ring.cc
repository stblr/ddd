#include <common/Ring.hh>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

TEST_CASE("Ring") {
    SUBCASE("Empty") {
        Ring<u32, 8> ring;

        SUBCASE("empty") {
            CHECK(ring.empty());
        }

        SUBCASE("full") {
            CHECK_FALSE(ring.full());
        }

        SUBCASE("count") {
            CHECK(ring.count() == 0);
        }

        SUBCASE("front") {
            CHECK(ring.front() == nullptr);
        }

        SUBCASE("back") {
            CHECK(ring.back() == nullptr);
        }

        SUBCASE("pushFront") {
            CHECK(ring.pushFront(96));
            CHECK(*ring.front() == 96);
        }

        SUBCASE("pushBack") {
            CHECK(ring.pushBack(168));
            CHECK(*ring.back() == 168);
        }

        SUBCASE("popFront") {
            CHECK_FALSE(ring.popFront());
        }

        SUBCASE("popBack") {
            CHECK_FALSE(ring.popBack());
        }

        SUBCASE("reset") {
            ring.reset();
            CHECK(ring.empty());
        }
    }

    SUBCASE("Full") {
        Ring<u32, 8> ring;
        ring.pushBack(231);
        ring.pushBack(67);
        ring.pushBack(102);
        ring.pushBack(207);
        ring.pushBack(211);
        ring.pushBack(56);
        ring.pushBack(84);
        ring.pushBack(220);

        SUBCASE("empty") {
            CHECK_FALSE(ring.empty());
        }

        SUBCASE("full") {
            CHECK(ring.full());
        }

        SUBCASE("count") {
            CHECK(ring.count() == 8);
        }

        SUBCASE("front") {
            CHECK(*ring.front() == 231);
        }

        SUBCASE("back") {
            CHECK(*ring.back() == 220);
        }

        SUBCASE("operator[]") {
            CHECK(ring[6] == 84);
        }

        SUBCASE("pushFront") {
            CHECK_FALSE(ring.pushFront(100));
        }

        SUBCASE("pushBack") {
            CHECK_FALSE(ring.pushBack(106));
        }

        SUBCASE("popFront") {
            CHECK(ring.popFront());
            CHECK(*ring.front() == 67);
        }

        SUBCASE("popBack") {
            CHECK(ring.popBack());
            CHECK(*ring.back() == 84);
        }

        SUBCASE("swapRemoveFront") {
            ring.swapRemoveFront(5);
            CHECK(*ring.front() == 67);
            CHECK(ring[4] == 231);
        }

        SUBCASE("swapRemoveBack") {
            ring.swapRemoveBack(5);
            CHECK(*ring.back() == 84);
            CHECK(ring[5] == 220);
        }

        SUBCASE("reset") {
            ring.reset();
            CHECK(ring.empty());
        }
    }

    SUBCASE("Wrapped around") {
        Ring<u32, 8> ring;
        for (size_t i = 0; i < 6; i++) {
            ring.pushBack(115);
            ring.popFront();
        }
        ring.pushBack(70);
        ring.pushBack(32);
        ring.pushBack(123);

        SUBCASE("empty") {
            CHECK_FALSE(ring.empty());
        }

        SUBCASE("full") {
            CHECK_FALSE(ring.full());
        }

        SUBCASE("count") {
            CHECK(ring.count() == 3);
        }

        SUBCASE("front") {
            CHECK(*ring.front() == 70);
        }

        SUBCASE("back") {
            CHECK(*ring.back() == 123);
        }

        SUBCASE("operator[]") {
            CHECK(ring[1] == 32);
        }

        SUBCASE("pushFront") {
            CHECK(ring.pushFront(111));
            CHECK(*ring.front() == 111);
        }

        SUBCASE("pushBack") {
            CHECK(ring.pushBack(104));
            CHECK(*ring.back() == 104);
        }

        SUBCASE("popFront") {
            CHECK(ring.popFront());
            CHECK(*ring.front() == 32);
        }

        SUBCASE("popBack") {
            CHECK(ring.popBack());
            CHECK(*ring.back() == 32);
        }

        SUBCASE("swapRemoveFront") {
            ring.swapRemoveFront(1);
            CHECK(*ring.front() == 70);
        }

        SUBCASE("swapRemoveBack") {
            ring.swapRemoveBack(1);
            CHECK(*ring.back() == 123);
        }

        SUBCASE("reset") {
            ring.reset();
            CHECK(ring.empty());
        }
    }
}
