module;

#include <streambuf>

export module core.concepts;

export namespace Core::Concepts {
    template <typename T>
    concept WritableStream = requires(T& stream, const char* data, std::streamsize n) {
        stream.write(data, n);
    };
}