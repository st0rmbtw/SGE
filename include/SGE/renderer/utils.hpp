#ifndef SGE_RENDERER_UTILS_HPP_
#define SGE_RENDERER_UTILS_HPP_

#include <LLGL/Buffer.h>
#include <LLGL/CommandBuffer.h>

namespace sge {

/**
 * @brief Copies the data to the buffer in chunks the size of maximum `2^16 - 1` (`65535`) bytes each.
 * 
 * @param commandBuffer The command buffer.
 * @param buffer The destination buffer.
 * @param offset The offset from the start of the data;
 * @param data The pointer to the data.
 * @param length The total length of bytes to copy.
 */

inline void UpdateBufferChunked(LLGL::CommandBuffer& commandBuffer, LLGL::Buffer& buffer, size_t offset, const void* data, size_t length) {
    static constexpr size_t MAX_BATCH_UPDATE_SIZE = (1u << 16u) - 1u;

    const uint8_t* dataPtr = static_cast<const uint8_t*>(data);

    while (length >= MAX_BATCH_UPDATE_SIZE) {
        commandBuffer.UpdateBuffer(buffer, offset, dataPtr + offset, MAX_BATCH_UPDATE_SIZE);
        offset += MAX_BATCH_UPDATE_SIZE;
        length -= MAX_BATCH_UPDATE_SIZE;
    }

    if (length > 0)
        commandBuffer.UpdateBuffer(buffer, offset, dataPtr + offset, length);
}

template <typename Container>
inline constexpr std::size_t GetArraySize(const Container& container) noexcept {
    return (container.size() * sizeof(typename Container::value_type));
}

template <typename T, std::size_t N>
inline consteval std::size_t GetArraySize(const T (&)[N]) noexcept {
    return (N * sizeof(T));
}

template <typename T, std::size_t N>
inline consteval std::size_t GetArrayLength(const T (&)[N]) noexcept {
    return N;
}

};

#endif // SGE_RENDERER_UTILS_HPP_
