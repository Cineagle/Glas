export module Glas.LinesEntry;
export import Glas.Entry;
export import Glas.OutputManager;
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
    concept LinesEntryMixins =
        std::is_class_v<T> &&
        std::default_initializable<T> &&
        std::is_copy_constructible_v<T>;


    template <LinesEntryMixins... Mixins>
    class LinesEntry :
        public Entry,
        public OutputManager<>,
        public Mixins...
    {
    public:
        LinesEntry();

        LinesEntry(const LinesEntry&) = default;
        virtual ~LinesEntry() override = default;
    private:
        LinesEntry& operator=(const LinesEntry&) = delete;
        LinesEntry(LinesEntry&&) noexcept = delete;
        LinesEntry& operator=(LinesEntry&&) noexcept = delete;
    public:
        void lines(this auto& self, const std::size_t count,
            const std::source_location location = std::source_location::current());
    private:
        virtual void expose() & override;
    private:
        auto prepare(this auto& self, const std::size_t count, const std::source_location& location);
        void deliver(this auto& self, std::unique_ptr<LinesEntry<Mixins...>>&& entry);
        auto format() &;
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
        static constexpr std::string_view type{ "Lines" };
    private:
        std::string vtBegin;
    };
}

export namespace Glas
{
    template <LinesEntryMixins... Mixins>
    LinesEntry<Mixins...>::LinesEntry() {
        vtBegin = VTSequence::formVTBegin(style);
    }

    template <LinesEntryMixins... Mixins>
    void LinesEntry<Mixins...>::lines(this auto& self, const std::size_t count,
        const std::source_location location)
    {
        self.LinesEntry<Mixins...>::deliver(
            self.LinesEntry<Mixins...>::prepare(
                count, location)
        );
    }

    template <LinesEntryMixins... Mixins>
    auto LinesEntry<Mixins...>::prepare(this auto& self, const std::size_t count,
        const std::source_location& location)
    {
        const std::string message(count, '\n');

        auto entry = std::make_unique<LinesEntry<Mixins...>>(self);

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

    template <LinesEntryMixins... Mixins>
    void LinesEntry<Mixins...>::deliver(this auto& self, std::unique_ptr<LinesEntry<Mixins...>>&& entry) {
        if (entry->Entry::outputScheme.load(std::memory_order_relaxed) == Scheme::Queue) {
            self.enqueue(std::move(entry));
        }
        else {
            entry->expose();
        }
    }

    template <LinesEntryMixins... Mixins>
    void LinesEntry<Mixins...>::expose() & {
        this->OutputManager::output(format());
    }

    template <LinesEntryMixins... Mixins>
    auto LinesEntry<Mixins...>::format() & {
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