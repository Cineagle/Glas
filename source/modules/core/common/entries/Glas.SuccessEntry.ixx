export module Glas.SuccessEntry;
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
    concept SuccessEntryMixins =
        std::is_class_v<T> &&
        std::default_initializable<T> &&
        std::is_copy_constructible_v<T>;


    template <SuccessEntryMixins... Mixins>
    class SuccessEntry :
        public Entry,
        public OutputManager<>,
        public Mixins...
    {
    public:
        SuccessEntry();

        SuccessEntry(const SuccessEntry&) = default;
        virtual ~SuccessEntry() override = default;
    private:
        SuccessEntry& operator=(const SuccessEntry&) = delete;
        SuccessEntry(SuccessEntry&&) noexcept = delete;
        SuccessEntry& operator=(SuccessEntry&&) noexcept = delete;
    public:
        void success(this auto& self, StringLike auto&& message,
            const std::source_location location = std::source_location::current());

        void success(this auto& self, Format format,
            const std::source_location location = std::source_location::current());
    private:
        virtual void expose() & override;
    private:
        auto prepare(this auto& self, StringLike auto&& message, const std::source_location& location);
        void deliver(this auto& self, std::unique_ptr<SuccessEntry<Mixins...>>&& entry);
        auto format() &;
        void output(std::vector<StringOutputFormat>&& formatted) const &;
    private:
        static constexpr VTStyle style{
            .fgColor{
                .red { 150 },
                .green{ 255 },
                .blue{ 150 }
            },
            .bgColor{
                .red { 12 },
                .green{ 12 },
                .blue{ 12 }
            }
        };
        static constexpr std::string_view type{ "Success" };
    private:
        std::string vtBegin;
    };
}

export namespace Glas
{
    template <SuccessEntryMixins... Mixins>
    SuccessEntry<Mixins...>::SuccessEntry() {
        vtBegin = VTSequence::formVTBegin(style);
    }

    template <SuccessEntryMixins... Mixins>
    void SuccessEntry<Mixins...>::success(this auto& self, StringLike auto&& message, 
        const std::source_location location)
    {
        self.SuccessEntry<Mixins...>::deliver(
            self.SuccessEntry<Mixins...>::prepare(
                std::forward<decltype(message)>(message), location)
        );
    }

    template <SuccessEntryMixins... Mixins>
    void SuccessEntry<Mixins...>::success(this auto& self, Format format,
        const std::source_location location)
    {
        self.SuccessEntry<Mixins...>::success(format.text, location);
    }

    template <SuccessEntryMixins... Mixins>
    auto SuccessEntry<Mixins...>::prepare(this auto& self, StringLike auto&& message,
        const std::source_location& location)
    {
        if constexpr (std::is_pointer_v<std::remove_cvref_t<decltype(message)>>) {
            if (!message) {
                throw Exception{ "`message` is nullptr." };
            }
        }

        auto entry = std::make_unique<SuccessEntry<Mixins...>>(self);

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

    template <SuccessEntryMixins... Mixins>
    void SuccessEntry<Mixins...>::deliver(this auto& self, 
        std::unique_ptr<SuccessEntry<Mixins...>>&& entry)
    {
        if (entry->Entry::outputScheme.load(std::memory_order_relaxed) == Scheme::Queue) {
            self.enqueue(std::move(entry));
        }
        else {
            entry->expose();
        }
    }

    template <SuccessEntryMixins... Mixins>
    void SuccessEntry<Mixins...>::expose()& {
        output(format());
    }

    template <SuccessEntryMixins... Mixins>
    auto SuccessEntry<Mixins...>::format() & {
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

    template <SuccessEntryMixins... Mixins>
    void SuccessEntry<Mixins...>::output(std::vector<StringOutputFormat>&& formatted) const & {
        const auto outputs = this->OutputManager::sharedOutputs.load(std::memory_order_relaxed);
        if (outputs) {
            for (const auto& output : *outputs) {
                output->output(formatted);
            }
        }
    }
}