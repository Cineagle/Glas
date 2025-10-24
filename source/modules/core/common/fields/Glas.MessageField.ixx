export module Glas.MessageField;
export import Glas.VTSequence;
export import Glas.StringOutputFormat;
export import Glas.Padding;
export import Glas.MessagePrepare;
export import Glas.Exception;
export import std;


export namespace Glas
{
    template <typename... Ts>
    concept MessageFieldSupport =
        (std::same_as<std::remove_cvref_t<Ts>, MessagePrepare> || ...);


    class MessageField :
        public PaddingProperty,
        public VTSequence
    {
    public:
        MessageField(const MessageField& other);

        MessageField() = default;
        virtual ~MessageField() = default;
    private:
        MessageField& operator=(const MessageField&) = delete;
        MessageField(MessageField&&) noexcept = delete;
        MessageField& operator=(MessageField&&) noexcept = delete;
    public:
        void enable() & noexcept;
        void disable() & noexcept;
    protected:
        template <typename... Ts>
            requires MessageFieldSupport<Ts...>
        void prepare(Ts&&... args) &;

        StringOutputFormat format(const std::string_view entryVTBegin) const &;
    protected:
        std::atomic<bool> enabled{};
    private:
        std::optional<std::string> message;
    };
}

export namespace Glas
{
    MessageField::MessageField(const MessageField& other) :
        PaddingProperty{ other },
        VTSequence{ other }
    {
        enabled.store(other.enabled.load(std::memory_order_relaxed), std::memory_order_relaxed);
    }

    void MessageField::enable() & noexcept {
        enabled.store(true, std::memory_order_relaxed);
    }

    void MessageField::disable() & noexcept {
        enabled.store(false, std::memory_order_relaxed);
    }

    template <typename... Ts>
        requires MessageFieldSupport<Ts...>
    void MessageField::prepare(Ts&&... args) & {
        const auto unpacker = [&]<typename T>(T&& arg) {
            if constexpr (std::same_as<std::remove_cvref_t<T>, MessagePrepare>) {
                message.emplace(arg.message);
            }
        };
        
        ((unpacker.operator()<Ts>(std::forward<Ts>(args))), ...);
    }

    StringOutputFormat MessageField::format(const std::string_view entryVTBegin) const & {
        std::string text;

        const auto paddingSnapshot = std::atomic_load_explicit(&paddingStrings,
            std::memory_order_acquire);

        if (paddingSnapshot) {
            text.append(paddingSnapshot->breakBefore);
            text.append(paddingSnapshot->spaceBefore);
        }

        text.append(message.value());

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