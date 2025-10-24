export module Glas.SequenceEntry;
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
    concept SequenceEntryMixins =
        std::is_class_v<T> &&
        std::default_initializable<T> &&
        std::is_copy_constructible_v<T>;


    template <SequenceEntryMixins... Mixins>
    class SequenceEntry :
        public Entry,
        public OutputManager<>,
        public Mixins...
    {
    public:
        SequenceEntry();

        SequenceEntry(const SequenceEntry&) = default;
        virtual ~SequenceEntry() override = default;
    private:
        SequenceEntry& operator=(const SequenceEntry&) = delete;
        SequenceEntry(SequenceEntry&&) noexcept = delete;
        SequenceEntry& operator=(SequenceEntry&&) noexcept = delete;
    public:
        void sequence(this auto& self, const std::size_t count, 
            Glas::StringLike auto&& message,
            const std::source_location location = std::source_location::current());

        void sequence(this auto& self, const std::size_t count, Format format,
            const std::source_location location = std::source_location::current());

        void sequence(this auto& self, const std::size_t count, const char symbol,
            const std::source_location location = std::source_location::current());
    private:
        static std::string buildSequence(const std::size_t count, const std::string_view view);
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
        static constexpr std::string_view type{ "Sequence" };
    private:
        std::string vtBegin;
    };
}

export namespace Glas
{
    template <SequenceEntryMixins... Mixins>
    SequenceEntry<Mixins...>::SequenceEntry() {
        vtBegin = VTSequence::formVTBegin(style);
    }

    template <SequenceEntryMixins... Mixins>
    void SequenceEntry<Mixins...>::sequence(this auto& self, const std::size_t count,
        Glas::StringLike auto&& message, const std::source_location location)
    {
        if constexpr (std::is_pointer_v<std::remove_cvref_t<decltype(message)>>) {
            if (!message) {
                throw Exception{ "`message` is nullptr." };
            }
        }

        auto entry = std::make_unique<SequenceEntry<Mixins...>>(self);

        const auto repeated = buildSequence(count, std::string_view{ message });

        const auto unpacker = [&]<typename T>() {
            if (entry->T::enabled.load(std::memory_order_relaxed)) {
                entry->T::prepare(
                    TypePrepare{ type },
                    LoggerNamePrepare{ self.loggerName },
                    MessagePrepare{ repeated },
                    LocationPrepare{ location }
                );
            }
        };

        ((unpacker.operator()<Mixins>()), ...);

        if (self.SequenceEntry<Mixins...>::Entry::outputScheme.load(std::memory_order_relaxed) ==
            Scheme::Queue) 
        {
            self.enqueue(std::move(entry));
        }
        else {
            entry->expose();
        }
    }

    template <SequenceEntryMixins... Mixins>
    void SequenceEntry<Mixins...>::sequence(this auto& self, const std::size_t count, 
        Format format, const std::source_location location)
    {
        self.sequence(count, format.text, location);
    }

    template <SequenceEntryMixins... Mixins>
    void SequenceEntry<Mixins...>::sequence(this auto& self, const std::size_t count,
        const char symbol, const std::source_location location)
    {
        self.sequence(count, std::string{ symbol }, location);
    }

    template <SequenceEntryMixins... Mixins>
    std::string SequenceEntry<Mixins...>::buildSequence(const std::size_t count, 
        const std::string_view view) 
    {
        std::string result;

        result.resize_and_overwrite(view.size() * count, [&](char* data, const size_t size) -> size_t {
            auto ptr = data;
            for (size_t i{}; i < count; ++i) {
                ptr = std::copy(view.begin(), view.end(), ptr);
            }
            return size;
        });

        return result;
    }

    template <SequenceEntryMixins... Mixins>
    void SequenceEntry<Mixins...>::expose() & {
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