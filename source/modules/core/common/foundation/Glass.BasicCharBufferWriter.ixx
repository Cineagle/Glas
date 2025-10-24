export module Glas.BasicCharBufferWriter;
export import Glas.Concepts;
export import std;


export namespace Glas
{
    template <typename CharT>
    class BasicCharBufferWriter {
    public:
        using StringView = std::basic_string_view<CharT>;
        enum class Result { Success, Truncated, Empty };
    public:
        template <std::size_t N>
        explicit BasicCharBufferWriter(CharT(&target)[N]) noexcept;

        BasicCharBufferWriter(CharT const* target, const std::size_t capacity) noexcept;
    public:
        template <StringLikeT<CharT>  T>
        [[nodiscard]] Result assign(const T& source) noexcept;

        template <StringLikeT<CharT>  T>
        [[nodiscard]] Result operator=(const T& source) noexcept;

        std::wstring_view view() const noexcept;
        operator std::wstring_view() const noexcept;

        std::size_t capacity() const noexcept;

        void clear() noexcept;
        void operator=(std::nullptr_t) noexcept;
    private:
        std::span<CharT> buffer;
    };


    using CharBufferWriter = BasicCharBufferWriter<char>;
    using WCharBufferWriter = BasicCharBufferWriter<wchar_t>;
}

export namespace Glas
{
    template <typename CharT>
    template <std::size_t N>
    BasicCharBufferWriter<CharT>::BasicCharBufferWriter(CharT(&target)[N]) noexcept :
        buffer{ target, N }
    {}

    template <typename CharT>
    BasicCharBufferWriter<CharT>:: BasicCharBufferWriter(CharT const* target, const std::size_t capacity) noexcept :
        buffer{ target, capacity }
    {}

    template <typename CharT>
    template <StringLikeT<CharT>  T>
    [[nodiscard]] BasicCharBufferWriter<CharT>::Result BasicCharBufferWriter<CharT>::assign(const T& source) noexcept {
        if (buffer.empty()) {
            return Result::Empty;
        }

        if constexpr (std::is_pointer_v<T>) {
            if (!source) {
                buffer[0] = CharT{};
                return Result::Empty;
            }
        }

        StringView view{ source };
        if (view.empty()) {
            buffer[0] = CharT{};
            return Result::Empty;
        }

        std::size_t count{ (std::min)(view.size(), buffer.size() - 1) };
        std::memcpy(buffer.data(), view.data(), count * sizeof(CharT));
        buffer[count] = CharT{};

        return (view.size() > count) ? Result::Truncated : Result::Success;
    }

    template <typename CharT>
    template <StringLikeT<CharT>  T>
    [[nodiscard]] BasicCharBufferWriter<CharT>::Result BasicCharBufferWriter<CharT>::operator=(const T& source) noexcept {
        return assign(source);
    }

    template <typename CharT>
    std::wstring_view BasicCharBufferWriter<CharT>::view() const noexcept {
        return buffer.empty() ? std::wstring_view{} : std::wstring_view{ buffer.data() };
    }

    template <typename CharT>
    BasicCharBufferWriter<CharT>::operator std::wstring_view() const noexcept {
        return view();
    }

    template <typename CharT>
    std::size_t BasicCharBufferWriter<CharT>::capacity() const noexcept {
        return buffer.size();
    }

    template <typename CharT>
    void BasicCharBufferWriter<CharT>::clear() noexcept {
        if (!buffer.empty()) {
            buffer[0] = CharT{};
        }
    }

    template <typename CharT>
    void BasicCharBufferWriter<CharT>::operator=(std::nullptr_t) noexcept {
        clear();
    }
}