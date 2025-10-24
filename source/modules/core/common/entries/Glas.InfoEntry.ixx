export module Glas.InfoEntry;
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
    concept InfoEntryMixins =
        std::is_class_v<T> &&
        std::default_initializable<T> &&
        std::is_copy_constructible_v<T>;


    template <InfoEntryMixins... Mixins>
    class InfoEntry :
        public Entry,
        public OutputManager<>,
        public Mixins...
    {
    public:
        InfoEntry();

        InfoEntry(const InfoEntry&) = default;
        virtual ~InfoEntry() override = default;
    private:
        InfoEntry& operator=(const InfoEntry&) = delete;
        InfoEntry(InfoEntry&&) noexcept = delete;
        InfoEntry& operator=(InfoEntry&&) noexcept = delete;
    public:
        void info(this auto& self, StringLike auto&& message,
            const std::source_location location = std::source_location::current());

        void info(this auto& self, Format format,
            const std::source_location location = std::source_location::current());
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
        static constexpr std::string_view type{ "Info" };
    private:
        std::string vtBegin;
    };
}

export namespace Glas
{
    template <InfoEntryMixins... Mixins>
    InfoEntry<Mixins...>::InfoEntry() {
        vtBegin = VTSequence::formVTBegin(style);
    }

    template <InfoEntryMixins... Mixins>
    void InfoEntry<Mixins...>::info(this auto& self,
        StringLike auto&& message, const std::source_location location)
    {
        if constexpr (std::is_pointer_v<std::remove_cvref_t<decltype(message)>>) {
            if (!message) {
                throw Exception{ "`message` is nullptr." };
            }
        }

        auto entry = std::make_unique<InfoEntry<Mixins...>>(self);

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

        if (self.InfoEntry<Mixins...>::Entry::outputScheme.load(std::memory_order_relaxed) == 
            Scheme::Queue) 
        {
            self.enqueue(std::move(entry));
        }
        else {
            entry->expose();
        }
    }

    template <InfoEntryMixins... Mixins>
    void InfoEntry<Mixins...>::info(this auto& self, Format format,
        const std::source_location location)
    {
        self.info(format.text, location);
    }

    template <InfoEntryMixins... Mixins>
    void InfoEntry<Mixins...>::expose() & {
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