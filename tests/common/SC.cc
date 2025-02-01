#include <common/SC.hh>
#include <common/ios/File.hh>
#include <common/ios/Resource.hh>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <cstring>
#include <optional>
#include <vector>

static std::optional<std::vector<u8>> file;

namespace IOS {

Resource::Resource(const char *path, u32 mode, bool /* isBlocking */) : m_fd(-1) {
    open(path, mode);
}

Resource::~Resource() = default;

s32 Resource::open(const char *path, u32 /* mode */) {
    m_fd = !strncmp(path, "/shared2/sys/SYSCONF", strlen("/shared2/sys/SYSCONF")) && file ? 0 : -1;
    return m_fd >= 0;
}

s32 File::read(void *output, u32 outputSize) {
    if (!output || !file) {
        return -4;
    }

    if (outputSize > file->size()) {
        outputSize = file->size();
    }
    memcpy(output, file->data(), outputSize);
    return outputSize;
}

} // namespace IOS

TEST_CASE("SC") {
    SUBCASE("No file") {
        file.reset();

        SC sc;

        SUBCASE("ok") {
            CHECK_FALSE(sc.ok());
        }

        SUBCASE("get") {
            u32 value;
            CHECK_FALSE(sc.get("My value", value));
        }
    }

    SUBCASE("Invalid file header") {
        file.emplace(0x4000);

        SC sc;

        SUBCASE("ok") {
            CHECK_FALSE(sc.ok());
        }

        SUBCASE("get") {
            u32 value;
            CHECK_FALSE(sc.get("My value", value));
            CHECK_FALSE(sc.get("My other value", value));
        }
    }

    SUBCASE("Invalid file contents") {
        file.emplace(0x4000);
        memcpy(file->data() + 0x0, "SCv0", strlen("SCv0"));

        SC sc;

        SUBCASE("ok") {
            CHECK(sc.ok());
        }

        SUBCASE("get") {
            u32 value;
            CHECK_FALSE(sc.get("My value", value));
            CHECK_FALSE(sc.get("My other value", value));
        }
    }

    SUBCASE("Valid file") {
        file.emplace(0x4000);
        memcpy(file->data() + 0x0, "SCv0", strlen("SCv0"));
        Bytes::WriteBE<u16>(file->data(), 0x4, 0x1);
        Bytes::WriteBE<u16>(file->data(), 0x6, 0x8);
        Bytes::WriteBE<u8>(file->data(), 0x8, 5 << 5 | (strlen("My value") - 1) << 0);
        memcpy(file->data() + 0x9, "My value", strlen("My value"));
        Bytes::WriteBE<u32>(file->data(), 0x9 + strlen("My value"), 42);

        SC sc;

        SUBCASE("ok") {
            CHECK(sc.ok());
        }

        SUBCASE("get") {
            u32 value;
            CHECK(sc.get("My value", value));
            CHECK(value == 42);
            CHECK_FALSE(sc.get("My other value", value));
        }
    }
}
