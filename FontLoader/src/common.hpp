#ifndef COMMON_HPP_
#define COMMON_HPP_

#include <cassert>
#include <fstream>
#include <string>
#include <vector>

namespace common {

inline constexpr float ApproxEquals(float a, float b, float eps = 0.001f) noexcept {
    return std::abs(a-b) < eps;
}

template <typename T>
inline constexpr size_t GetVectorByteSize(const std::vector<T>& v) {
    return v.size() * sizeof(v[0]);
}

inline void ReadEntireFile(const std::string& path, std::vector<uint8_t>& outBuffer) {
    std::ifstream file(path, std::ios::binary);
    assert(!file.fail());

    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    outBuffer.resize(size);
    bool failed = file.read(reinterpret_cast<char*>(outBuffer.data()), size).fail();
    assert(!failed);
}

} // namespace common

#endif // COMMON_HPP_