export module Glas.TraceEntry;
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
    concept TraceEntryMixins =
        std::is_class_v<T> &&
        std::default_initializable<T> &&
        std::is_copy_constructible_v<T>;


    template <TraceEntryMixins... Mixins>
    class TraceEntry :
        public Entry,
        public OutputManager<>,
        public Mixins...
    {
    public:
        TraceEntry();

        TraceEntry(const TraceEntry&) = default;
        virtual ~TraceEntry() override = default;
    private:
        TraceEntry& operator=(const TraceEntry&) = delete;
        TraceEntry(TraceEntry&&) noexcept = delete;
        TraceEntry& operator=(TraceEntry&&) noexcept = delete;
    public:
        void trace(this auto& self, StringLike auto&& message,
            const std::source_location location = std::source_location::current());

        void trace(this auto& self, Format format,
            const std::source_location location = std::source_location::current());

        void trace(this auto& self,
            const std::source_location location = std::source_location::current());
    private:
        static std::string formTrace();
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
        static constexpr std::string_view type{ "Trace" };
    private:
        std::string vtBegin;
    };
}

export namespace Glas
{
    template <TraceEntryMixins... Mixins>
    TraceEntry<Mixins...>::TraceEntry() {
        vtBegin = VTSequence::formVTBegin(style);
    }

    template <TraceEntryMixins... Mixins>
    void TraceEntry<Mixins...>::trace(this auto& self, StringLike auto&& message, 
        const std::source_location location)
    {
        if constexpr (std::is_pointer_v<std::remove_cvref_t<decltype(message)>>) {
            if (!message) {
                throw Exception{ "`message` is nullptr." };
            }
        }

        auto entry = std::make_unique<TraceEntry<Mixins...>>(self);

        std::string formed{ std::forward<decltype(message)>(message) };
        formed.append(formTrace());

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

        if (entry->Entry::outputScheme.load(std::memory_order_relaxed) == Scheme::Queue) {
            self.enqueue(std::move(entry));
        }
        else {
            entry->expose();
        }
    }

    template <TraceEntryMixins... Mixins>
    void TraceEntry<Mixins...>::trace(this auto& self, Format format,
        const std::source_location location)
    {
        self.trace(format.text, location);
    }

    template <TraceEntryMixins... Mixins>
    void TraceEntry<Mixins...>::trace(this auto& self, const std::source_location location)
    {
        self.trace("", location);
    }

    template <TraceEntryMixins... Mixins>
    std::string TraceEntry<Mixins...>::formTrace() {
        std::string text;
#ifdef NDEBUG
        text.append("-----------------------------------------------");
        text.append("\n`StackTrace not captured in Release build.`");
        text.append("\n-----------------------------------------------");
#else
        for (const auto& entry : std::stacktrace::current() | std::views::drop(4)) {
            text.append("-----------------------------------------------");

            text.append("\n- File: `");
            text.append(entry.source_file());

            text.append("`\n- Line: ");
            text.append(std::to_string(entry.source_line()));

            text.append("\n- Function: `");
            text.append(entry.description());
            text.append("`\n");
        }
        text.append("-----------------------------------------------");
#endif
        return text;
    }

    template <TraceEntryMixins... Mixins>
    void TraceEntry<Mixins...>::expose() & {
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