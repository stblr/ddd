#include <common/DolphinVersion.hh>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <cstdio>
#include <cstring>

TEST_CASE("DolphinVersion") {
    Array<char, 64> versionString;
    DolphinVersion oldVersion{5, 0, 20288};
    DolphinVersion newVersion{2412, 0, 0};

    SUBCASE("Read") {
        SUBCASE("Invalid") {
            versionString.fill('1');

            DolphinVersion version;
            bool result = DolphinVersion::Read(versionString, version);

            CHECK_FALSE(result);
        }

        SUBCASE("Valid old") {
            versionString.fill('\0');
            snprintf(versionString.values(), versionString.count(), "%u.%u-%u", oldVersion.major,
                    oldVersion.minor, oldVersion.patch);

            DolphinVersion version;
            bool result = DolphinVersion::Read(versionString, version);

            CHECK(result);
            CHECK(version == oldVersion);
        }

        SUBCASE("Valid new") {
            versionString.fill('\0');
            snprintf(versionString.values(), versionString.count(), "%u", newVersion.major);

            DolphinVersion version;
            bool result = DolphinVersion::Read(versionString, version);

            CHECK(result);
            CHECK(version == newVersion);
        }
    }

    SUBCASE("operator==") {
        CHECK(oldVersion == oldVersion);
    }

    SUBCASE("operator!=") {
        CHECK(oldVersion != newVersion);
    }

    SUBCASE("operator<") {
        CHECK(oldVersion < newVersion);
    }

    SUBCASE("operator>") {
        CHECK(newVersion > oldVersion);
    }

    SUBCASE("operator<=") {
        CHECK(oldVersion <= oldVersion);
    }

    SUBCASE("operator>=") {
        CHECK(oldVersion >= oldVersion);
    }
}
