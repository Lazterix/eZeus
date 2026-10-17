#include "fileIO/ereadstream.h"

#include <array>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <vector>

namespace {
bool testVectorShortRead() {
    const std::vector<char> bytes{0x12, 0x34};
    eReadSource source(&bytes);
    std::uint32_t value = 0xffffffff;

    const auto read = source.read(&value, sizeof(value));

    return read == bytes.size() && value == 0 && source.failed();
}

bool testFileShortRead() {
    const auto suffix = std::chrono::steady_clock::now().time_since_epoch().count();
    const auto path = std::filesystem::temp_directory_path() /
                      ("ezeus-readsource-" + std::to_string(suffix) + ".bin");
    {
        std::ofstream file(path, std::ios::binary);
        const std::array<char, 2> bytes{0x12, 0x34};
        file.write(bytes.data(), bytes.size());
    }

    std::ifstream file(path, std::ios::binary);
    eReadSource source(&file);
    std::uint32_t value = 0xffffffff;
    const auto read = source.read(&value, sizeof(value));
    file.close();
    std::filesystem::remove(path);

    return read == 2 && value == 0 && source.failed();
}

bool testExactRead() {
    const std::vector<char> bytes{0x12, 0x34, 0x56, 0x78};
    eReadSource source(&bytes);
    std::array<char, 4> value{};

    const auto read = source.read(value.data(), value.size());

    return read == bytes.size() && value[0] == 0x12 && value[3] == 0x78 &&
           !source.failed();
}

bool testExplicitFailure() {
    const std::vector<char> bytes;
    eReadSource source(&bytes);

    source.fail();

    return source.failed();
}
}

int main() {
    return testVectorShortRead() && testFileShortRead() && testExactRead() &&
           testExplicitFailure() ? 0 : 1;
}
