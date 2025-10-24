export module Glas.LoggerNameField;
export import Glas.VTSequence;
export import Glas.StringOutputFormat;
export import Glas.Padding;
export import Glas.LoggerNamePrepare;
export import Glas.Exception;
export import std;


export namespace Glas
{
    template <typename... Ts>
    concept LoggerNameFieldPrepareSupport =
        (std::same_as<std::remove_cvref_t<Ts>, LoggerNamePrepare> || ...);


    class LoggerNameField :
        public PaddingProperty,
        public VTSequence
    {
    public:
        LoggerNameField(const LoggerNameField& other);

        LoggerNameField() = default;
        virtual ~LoggerNameField() = default;
    private:
        LoggerNameField& operator=(const LoggerNameField&) = delete;
        LoggerNameField(LoggerNameField&&) noexcept = delete;
        LoggerNameField& operator=(LoggerNameField&&) noexcept = delete;
    public:
        void enable() & noexcept;
        void disable() & noexcept;
    protected:
        template <typename... Ts>
            requires LoggerNameFieldPrepareSupport<Ts...>
        void prepare(Ts&&... args) &;

        StringOutputFormat format(const std::string_view entryVTBegin) const &;
    protected:
        std::atomic<bool> enabled{};
    private:
        std::optional<std::string_view> loggerName;
    };
}

export namespace Glas
{
    LoggerNameField::LoggerNameField(const LoggerNameField& other) :
        PaddingProperty{ other },
        VTSequence{ other }
    {
        enabled.store(other.enabled.load(std::memory_order_relaxed), std::memory_order_relaxed);
    }

    void LoggerNameField::enable() & noexcept {
        enabled.store(true, std::memory_order_relaxed);
    }

    void LoggerNameField::disable() & noexcept {
        enabled.store(false, std::memory_order_relaxed);
    }

    template <typename... Ts>
        requires LoggerNameFieldPrepareSupport<Ts...>
    void LoggerNameField::prepare(Ts&&... args) & {
        const auto unpacker = [&]<typename T>(T&& arg) {
            if constexpr (std::same_as<std::remove_cvref_t<T>, LoggerNamePrepare>) {
                loggerName.emplace(arg.loggerName);
            }
        };

        ((unpacker.operator()<Ts>(std::forward<Ts>(args))), ...);
    }

    StringOutputFormat LoggerNameField::format(const std::string_view entryVTBegin) const & {
        std::string text;

        const auto paddingSnapshot = std::atomic_load_explicit(&paddingStrings,
            std::memory_order_acquire);

        if (paddingSnapshot) {
            text.append(paddingSnapshot->breakBefore);
            text.append(paddingSnapshot->spaceBefore);
        }

        text.push_back('[');
        text.append(loggerName.value());
        text.push_back(']');

        if (paddingSnapshot) {
            text.append(paddingSnapshot->spaceAfter);
            text.append(paddingSnapshot->breakAfter);
        }

        StringOutputFormat formatted;

        if (VTSequence::enabled.load(std::memory_order_relaxed)) {
            const auto userVTBegin = vtBegin.load(std::memory_order_relaxed);
            if (userVTBegin) {
                formatted.vtBegin.emplace(std::move(*userVTBegin));
            }
            else {
                formatted.vtBegin.emplace(entryVTBegin);
            }
        }

        formatted.output = std::move(text);

        return formatted;
    }
}