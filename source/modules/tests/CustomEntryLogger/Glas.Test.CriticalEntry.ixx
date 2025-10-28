export module Glas.Test.CriticalEntry;
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


export namespace Glas::Test
{
    template <typename T>
    concept CriticalEntryMixins =
        std::is_class_v<T> &&
        std::default_initializable<T> &&
        std::is_copy_constructible_v<T>;


    template <CriticalEntryMixins... Mixins>
    class CriticalEntry :
        public Entry,
        public OutputManager<>,
        public Mixins...
    {
    public:
        CriticalEntry();

        CriticalEntry(const CriticalEntry&) = default;
        virtual ~CriticalEntry() override = default;
    private:
        CriticalEntry& operator=(const CriticalEntry&) = delete;
        CriticalEntry(CriticalEntry&&) noexcept = delete;
        CriticalEntry& operator=(CriticalEntry&&) noexcept = delete;
    public:
        void critical(this auto& self, StringLike auto&& message,
            const std::source_location location = std::source_location::current());

        void critical(this auto& self, Format format,
            const std::source_location location = std::source_location::current());
    private:
        virtual void expose() & override;
    private:
        auto prepare(this auto& self, StringLike auto&& message, const std::source_location& location);
        void deliver(this auto& self, std::unique_ptr<CriticalEntry<Mixins...>>&& entry);
        auto format() &;
    private:
        static constexpr VTStyle style{
            .fgColor{
                .red { 20 },
                .green{ 20 },
                .blue{ 20 }
            },
            .bgColor{
                .red { 220 },
                .green{ 100 },
                .blue{ 100 }
            }
        };
        static constexpr std::string_view type{ "Critical" };
    private:
        std::string vtBegin;
    };
}

export namespace Glas::Test
{
    template <CriticalEntryMixins... Mixins>
    CriticalEntry<Mixins...>::CriticalEntry() {
        vtBegin = VTSequence::formVTBegin(style);
    }

    template <CriticalEntryMixins... Mixins>
    void CriticalEntry<Mixins...>::critical(this auto& self, StringLike auto&& message, 
        const std::source_location location)
    {
        self.CriticalEntry<Mixins...>::deliver(
            self.CriticalEntry<Mixins...>::prepare(
                std::forward<decltype(message)>(message), location)
        );
    }

    template <CriticalEntryMixins... Mixins>
    void CriticalEntry<Mixins...>::critical(this auto& self, Format format,
        const std::source_location location)
    {
        self.CriticalEntry<Mixins...>::critical(format.text, location);
    }

    template <CriticalEntryMixins... Mixins>
    auto CriticalEntry<Mixins...>::prepare(this auto& self, StringLike auto&& message,
        const std::source_location& location)
    {
        if constexpr (std::is_pointer_v<std::remove_cvref_t<decltype(message)>>) {
            if (!message) {
                throw Exception{ "`message` is nullptr." };
            }
        }

        auto entry = std::make_unique<CriticalEntry<Mixins...>>(self);

        const auto unpacker = [&]<typename T>() {
            if (entry->T::enabled.load(std::memory_order_relaxed)) {
                entry->T::prepare(
                    TypePrepare{ type },
                    LoggerNamePrepare{ self.loggerName },
                    MessagePrepare{ message },
                    LocationPrepare{ location }
                );
            }
        };

        ((unpacker.operator()<Mixins>()), ...);

        return entry;
    }

    template <CriticalEntryMixins... Mixins>
    void CriticalEntry<Mixins...>::deliver(this auto& self, 
        std::unique_ptr<CriticalEntry<Mixins...>>&& entry) 
    {
        if (entry->Entry::outputScheme.load(std::memory_order_relaxed) == Scheme::Queue) {
            self.enqueue(std::move(entry));
        }
        else {
            entry->expose();
        }
    }

    template <CriticalEntryMixins... Mixins>
    void CriticalEntry<Mixins...>::expose() & {
        this->OutputManager::output(format());
    }

    template <CriticalEntryMixins... Mixins>
    auto CriticalEntry<Mixins...>::format() & {
        std::vector<StringOutputFormat> formatted;

        const auto unpacker = [&]<typename T>() {
            if (this->T::enabled.load(std::memory_order_relaxed)) {

                static_assert(
                    std::same_as<
                    std::remove_cvref_t<decltype(this->T::format(std::declval<std::string_view>()))>,
                    StringOutputFormat>,
                    "`format` return type mismatch."
                    );

                auto item = this->T::format(vtBegin);
                item.type.emplace(type);
                item.style.emplace(style);

                formatted.push_back(std::move(item));
            }
        };

        ((unpacker.operator()<Mixins>()), ...);

        return formatted;
    }
}