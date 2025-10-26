export module Glas.WarningEntry;
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
    concept WarningEntryMixins =
        std::is_class_v<T> &&
        std::default_initializable<T> &&
        std::is_copy_constructible_v<T>;


    template <WarningEntryMixins... Mixins>
    class WarningEntry :
        public Entry,
        public OutputManager<>,
        public Mixins...
    {
    public:
        WarningEntry();

        WarningEntry(const WarningEntry&) = default;
        virtual ~WarningEntry() override = default;
    private:
        WarningEntry& operator=(const WarningEntry&) = delete;
        WarningEntry(WarningEntry&&) noexcept = delete;
        WarningEntry& operator=(WarningEntry&&) noexcept = delete;
    public:
        void warning(this auto& self, StringLike auto&& message,
            const std::source_location location = std::source_location::current());

        void warning(this auto& self, Format format,
            const std::source_location location = std::source_location::current());
    private:
        virtual void expose() & override;
    private:
        auto prepare(this auto& self, StringLike auto&& message, const std::source_location& location);
        void deliver(this auto& self, std::unique_ptr<WarningEntry<Mixins...>>&& entry);
        auto format() &;
        void output(std::vector<StringOutputFormat>&& formatted) const &;
    private:
        static constexpr VTStyle style{
            .fgColor{
                .red { 250 },
                .green{ 193 },
                .blue{ 90 }
            },
            .bgColor{
                .red { 12 },
                .green{ 12 },
                .blue{ 12 }
            }
        };
        static constexpr std::string_view type{ "Warning" };
    private:
        std::string vtBegin;
    };
}

export namespace Glas
{
    template <WarningEntryMixins... Mixins>
    WarningEntry<Mixins...>::WarningEntry() {
        vtBegin = VTSequence::formVTBegin(style);
    }

    template <WarningEntryMixins... Mixins>
    void WarningEntry<Mixins...>::warning(this auto& self, StringLike auto&& message, 
        const std::source_location location)
    {
        self.WarningEntry<Mixins...>::deliver(
            self.WarningEntry<Mixins...>::prepare(
                std::forward<decltype(message)>(message), location)
        );
    }

    template <WarningEntryMixins... Mixins>
    void WarningEntry<Mixins...>::warning(this auto& self, Format format,
        const std::source_location location)
    {
        self.WarningEntry<Mixins...>::warning(format.text, location);
    }

    template <WarningEntryMixins... Mixins>
    auto WarningEntry<Mixins...>::prepare(this auto& self, StringLike auto&& message,
        const std::source_location& location)
    {
        if constexpr (std::is_pointer_v<std::remove_cvref_t<decltype(message)>>) {
            if (!message) {
                throw Exception{ "`message` is nullptr." };
            }
        }

        auto entry = std::make_unique<WarningEntry<Mixins...>>(self);

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

    template <WarningEntryMixins... Mixins>
    void WarningEntry<Mixins...>::deliver(this auto& self, 
        std::unique_ptr<WarningEntry<Mixins...>>&& entry)
    {
        if (entry->Entry::outputScheme.load(std::memory_order_relaxed) == Scheme::Queue) {
            self.enqueue(std::move(entry));
        }
        else {
            entry->expose();
        }
    }

    template <WarningEntryMixins... Mixins>
    void WarningEntry<Mixins...>::expose() & {
        output(format());
    }

    template <WarningEntryMixins... Mixins>
    auto WarningEntry<Mixins...>::format() & {
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

    template <WarningEntryMixins... Mixins>
    void WarningEntry<Mixins...>::output(std::vector<StringOutputFormat>&& formatted) const & {
        const auto outputs = this->OutputManager::sharedOutputs.load(std::memory_order_relaxed);
        if (outputs) {
            for (const auto& output : *outputs) {
                output->output(formatted);
            }
        }
    }
}