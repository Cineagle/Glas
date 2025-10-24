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
        public Glas::Entry,
        public Glas::OutputManager<>,
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
        void critical(this auto& self, Glas::StringLike auto&& message,
            const std::source_location location = std::source_location::current());

        void critical(this auto& self, Glas::Format format,
            const std::source_location location = std::source_location::current());
    private:
        virtual void expose() & override;
    private:
        static constexpr Glas::VTStyle style{
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
        vtBegin = Glas::VTSequence::formVTBegin(style);
    }

    template <CriticalEntryMixins... Mixins>
    void CriticalEntry<Mixins...>::critical(this auto& self,
        Glas::StringLike auto&& message, const std::source_location location)
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
                    Glas::TypePrepare{ type },
                    Glas::LoggerNamePrepare{ self.loggerName },
                    Glas::MessagePrepare{ message },
                    Glas::LocationPrepare{ location }
                );
            }
        };

        ((unpacker.operator()<Mixins>()), ...);

        if (self.CriticalEntry<Mixins...>::Entry::outputScheme.load(std::memory_order_relaxed) ==
            Scheme::Queue)
        {
            self.enqueue(std::move(entry));
        }
        else {
            entry->expose();
        }
    }

    template <CriticalEntryMixins... Mixins>
    void CriticalEntry<Mixins...>::critical(this auto& self, Glas::Format format,
        const std::source_location location)
    {
        self.critical(format.text, location);
    }

    template <CriticalEntryMixins... Mixins>
    void CriticalEntry<Mixins...>::expose() & {
        std::vector<Glas::StringOutputFormat> formatted;

        const auto unpacker = [&]<typename T>() {
            if (this->T::enabled.load(std::memory_order_relaxed)) {

                static_assert(
                    std::same_as<
                    std::remove_cvref_t<decltype(this->T::format(std::declval<std::string_view>()))>,
                    Glas::StringOutputFormat>,
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