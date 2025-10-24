export module Glas.TypeField;
export import Glas.VTSequence;
export import Glas.StringOutputFormat;
export import Glas.Padding;
export import Glas.TypePrepare;
export import Glas.Exception;
export import std;


export namespace Glas
{
    template <typename... Ts>
    concept TypeFieldSupport =
        (std::same_as<std::remove_cvref_t<Ts>, TypePrepare> || ...);


    class TypeField :
        public PaddingProperty,
        public VTSequence
    {
    public:
        TypeField(const TypeField& other);

        TypeField() = default;
        virtual ~TypeField() = default;
    private:
        TypeField& operator=(const TypeField&) = delete;
        TypeField(TypeField&&) noexcept = delete;
        TypeField& operator=(TypeField&&) noexcept = delete;
    public:
        void enable() & noexcept;
        void disable() & noexcept;
    protected:
        template <typename... Ts>
            requires TypeFieldSupport<Ts...>
        void prepare(Ts&&... args) &;

        StringOutputFormat format(const std::string_view entryVTBegin) const &;
    protected:
        std::atomic<bool> enabled{};
    private:
        std::optional<std::string_view> type;
    };
}

export namespace Glas
{
    TypeField::TypeField(const TypeField& other) :
        PaddingProperty{ other },
        VTSequence{ other }
    {
        enabled.store(other.enabled.load(std::memory_order_relaxed), std::memory_order_relaxed);
    }

    void TypeField::enable() & noexcept {
        enabled.store(true, std::memory_order_relaxed);
    }

    void TypeField::disable() & noexcept {
        enabled.store(false, std::memory_order_relaxed);
    }

    template <typename... Ts>
        requires TypeFieldSupport<Ts...>
    void TypeField::prepare(Ts&&... args) & {
        const auto unpacker = [&]<typename T>(T&& arg) {
            if constexpr (std::same_as<std::remove_cvref_t<T>, TypePrepare>) {
                type.emplace(arg.type);
            }
        };

        ((unpacker.operator()<Ts>(std::forward<Ts>(args))), ...);
    }

    StringOutputFormat TypeField::format(const std::string_view entryVTBegin) const & {
        std::string text;

        const auto paddingSnapshot = std::atomic_load_explicit(&paddingStrings,
            std::memory_order_acquire);

        if (paddingSnapshot) {
            text.append(paddingSnapshot->breakBefore);
            text.append(paddingSnapshot->spaceBefore);
        }

        text.push_back('[');
        text.append(type.value());
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