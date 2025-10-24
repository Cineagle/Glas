module;
#include <windows.h>

export module Glas.BytesEntry;
export import Glas.Entry;
export import Glas.OutputManager;
export import Glas.Concepts;
export import Glas.Format;
export import Glas.StringOutputFormat;
export import Glas.VTSequence;
export import Glas.TypePrepare;
export import Glas.LoggerNamePrepare;
export import Glas.MessagePrepare;
export import Glas.LocationPrepare;
export import std;


export namespace Glas
{
    template <typename T>
    concept BytesEntryMixins =
        std::is_class_v<T> &&
        std::default_initializable<T> &&
        std::is_copy_constructible_v<T>;


    template <BytesEntryMixins... Mixins>
    class BytesEntry :
        public Entry,
        public OutputManager<>,
        public Mixins...
    {
    private:
        using Protections = std::vector<std::tuple<void*, unsigned long, std::size_t>>;
        using Bytes = std::vector<std::byte>;
    public:
        BytesEntry();

        BytesEntry(const BytesEntry&) = default;
        virtual ~BytesEntry() override = default;
    private:
        BytesEntry& operator=(const BytesEntry&) = delete;
        BytesEntry(BytesEntry&&) noexcept = delete;
        BytesEntry& operator=(BytesEntry&&) noexcept = delete;
    public:
        void bytes(this auto& self, Glas::StringLike auto&& message,
            const void* address, const std::size_t count,
            const std::source_location location = std::source_location::current());

        void bytes(this auto& self, Format format, const void* address, 
            const std::size_t count,
            const std::source_location location = std::source_location::current());

        void bytes(this auto& self, const void* address, const std::size_t count,
            const std::source_location location = std::source_location::current());
    private:
        static std::string formBytes(const void* address, const std::size_t count);
        static Protections makeMemoryReadable(const void* address, const std::size_t count);
        static Bytes readMemory(const void* address, const std::size_t count);
        static void restoreMemoryProtections(const Protections& protections, const void* address,
            const std::size_t count);

        static  std::string convertToHexString(const Bytes& bytes, const std::size_t count);
    private:
        virtual void expose() & override;
    private:
        static constexpr VTStyle style{
            .fgColor{
                .red { 200 },
                .green{ 200 },
                .blue{ 200 }
            },
            .bgColor{
                .red { 12 },
                .green{ 12 },
                .blue{ 12 }
            }
        };
        static constexpr std::string_view type{ "Bytes" };
    private:
        std::string vtBegin;
    };
}

export namespace Glas
{
    template <BytesEntryMixins... Mixins>
    BytesEntry<Mixins...>::BytesEntry() {
        vtBegin = VTSequence::formVTBegin(style);
    }

    template <BytesEntryMixins... Mixins>
    void BytesEntry<Mixins...>::bytes(this auto& self, Glas::StringLike auto&& message,
        const void* address, const std::size_t count, const std::source_location location)
    {
        if constexpr (std::is_pointer_v<std::remove_cvref_t<decltype(message)>>) {
            if (!message) {
                throw Exception{ "`message` is nullptr." };
            }
        }

        if (!address) {
            throw Exception{ "`address` is nullptr." };
        }
        if (!count) {
            throw Exception{ "`count` is 0." };
        }

        auto entry = std::make_unique<BytesEntry<Mixins...>>(self);

        std::string formed{ std::forward<decltype(message)>(message) };
        formed.append(formBytes(address, count));

        const auto unpacker = [&]<typename T>() {
            if (entry->T::enabled.load(std::memory_order_relaxed)) {
                entry->T::prepare(
                    TypePrepare{ type },
                    LoggerNamePrepare{ self.loggerName },
                    MessagePrepare{ formed },
                    LocationPrepare{ location }
                );
            }
        };

        ((unpacker.operator()<Mixins>()), ...);

        if (self.BytesEntry<Mixins...>::Entry::outputScheme.load(std::memory_order_relaxed) ==
            Scheme::Queue) 
        {
            self.enqueue(std::move(entry));
        }
        else {
            entry->expose();
        }
    }

    template <BytesEntryMixins... Mixins>
    void BytesEntry<Mixins...>::bytes(this auto& self, Format format, const void* address,
        const std::size_t count, const std::source_location location)
    {
        self.bytes(format.text, address, count, location);
    }

    template <BytesEntryMixins... Mixins>
    void BytesEntry<Mixins...>::bytes(this auto& self, const void* address,
        const std::size_t count, const std::source_location location)
    {
        self.bytes("", address, count, location);
    }

    template <BytesEntryMixins... Mixins>
    std::string BytesEntry<Mixins...>::formBytes(const void* address, const std::size_t count) {
        const auto protections = makeMemoryReadable(address, count);
        const auto bytes = readMemory(address, count);
        restoreMemoryProtections(protections, address, count);
        return convertToHexString(bytes, count);
    }

    template <BytesEntryMixins... Mixins>
    BytesEntry<Mixins...>::Protections BytesEntry<Mixins...>::makeMemoryReadable(
        const void* address, const std::size_t count) 
    {
        Protections protections;

        const auto begin = reinterpret_cast<std::uintptr_t>(address);
        const auto end = begin + count;

        auto current = begin;
        while (current < end) {
            MEMORY_BASIC_INFORMATION mbi{};
            if (!VirtualQuery(reinterpret_cast<const void*>(current), &mbi, sizeof(mbi))) {
                throw Exception{ std::format("`VirtualQuery` failed at `current` address: {:p}. "
                    "`address`: {:p}. `count`: {}", reinterpret_cast<void*>(current), address, count) };
            }

            if (mbi.State != MEM_COMMIT) {
                throw Exception{ std::format("Memory not committed at `current` address: {:p}. "
                    "`address`: {:p}. `count`: {}", reinterpret_cast<void*>(current), address, count) };
            }

            const auto regionBegin = reinterpret_cast<std::uintptr_t>(mbi.BaseAddress);
            const auto regionEnd = regionBegin + mbi.RegionSize;
            const auto overlapSize = (std::min)(regionEnd, end) - (std::max)(regionBegin, begin);

            unsigned long protect{};
            const auto overlapBegin = reinterpret_cast<void*>((std::max)(regionBegin, begin));

            if (!VirtualProtect(overlapBegin, overlapSize, PAGE_EXECUTE_READWRITE, &protect)) {
                throw Exception{ std::format("`VirtualProtect` modify failed at `overlapBegin`: {:p}. "
                    "`overlapSize`: {}. `address`: {:p}. `count`: {}", overlapBegin, overlapSize, 
                    address, count) };
            }

            protections.emplace_back(overlapBegin, protect, overlapSize);

            current = regionEnd;
        }

        return protections;
    }

    template <BytesEntryMixins... Mixins>
    BytesEntry<Mixins...>::Bytes BytesEntry<Mixins...>::readMemory(const void* address, 
        const std::size_t count)
    {
        Bytes buffer(count);
        std::memcpy(buffer.data(), address, count);
        return buffer;
    }

    template <BytesEntryMixins... Mixins>
    void BytesEntry<Mixins...>::restoreMemoryProtections(const Protections& protections,
        const void* address, const std::size_t count)
    {
        for (auto [restoreAddress, restoreProtect, restoreSize] : protections) {
            if (!VirtualProtect(restoreAddress, restoreSize, restoreProtect, 
                &restoreProtect)) 
            {
                throw Exception{ 
                    std::format("`VirtualProtect` restore failed at `restoreAddress`: {:p}. "
                    "`restoreSize`: {}. `address`: {:p}. `count`: {}", restoreAddress, 
                        restoreSize, address, count) 
                };
            }
        }
    }

    template <BytesEntryMixins... Mixins>
    std::string BytesEntry<Mixins...>::convertToHexString(const Bytes& bytes, const std::size_t count) {
        std::string text(count * 3 - 1, '\0');

        char* out = text.data();
        static constexpr char HEX[]{ "0123456789ABCDEF" };

        for (std::size_t i{}; i < count; ++i) {
            if (i) {
                *out++ = ' ';
            }

            const auto byte = std::to_integer<unsigned char>(bytes[i]);
            *out++ = HEX[byte >> 4];
            *out++ = HEX[byte & 0x0F];
        }

        return text;
    }

    template <BytesEntryMixins... Mixins>
    void BytesEntry<Mixins...>::expose() & {
        std::vector<StringOutputFormat> formatted;

        const auto unpacker = [&]<typename T>() {
            if (this->T::enabled.load(std::memory_order_relaxed)) {

                static_assert(
                    std::same_as<
                    std::remove_cvref_t<decltype(this->T::format(std::declval<std::string_view>()))>,
                    StringOutputFormat>,
                    "`format` return type mismatch."
                );

                auto&& item = this->T::format(vtBegin);
                item.type.emplace(type);
                item.style.emplace(style);

                formatted.push_back(std::move(item));
            }
        };

        ((unpacker.operator()<Mixins>()), ...);

        const auto outputs = std::atomic_load_explicit(&this->OutputManager::sharedOutputs,
            std::memory_order_relaxed);

        if (outputs) {
            for (const auto& output : *outputs) {
                output->output(formatted);
            }
        }
    }
}