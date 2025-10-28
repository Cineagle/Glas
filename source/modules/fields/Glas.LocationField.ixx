export module Glas.LocationField;
export import Glas.VTSequence;
export import Glas.StringOutputFormat;
export import Glas.Padding;
export import Glas.LocationPrepare;
export import Glas.Exception;
export import std;


export namespace Glas
{
    template <typename... Ts>
    concept LocationFieldPrepareSupport =
        (std::same_as<std::remove_cvref_t<Ts>, LocationPrepare> || ...);


    class LocationField :
        public PaddingProperty,
        public VTSequence
    {
    public:
        LocationField(const LocationField& other);

        LocationField() = default;
        virtual ~LocationField() = default;
    private:
        LocationField& operator=(const LocationField&) = delete;
        LocationField(LocationField&&) noexcept = delete;
        LocationField& operator=(LocationField&&) noexcept = delete;
    public:
        void enable() & noexcept;
        void disable() & noexcept;
    protected:
        template <typename... Ts>
            requires LocationFieldPrepareSupport<Ts...>
        void prepare(Ts&&... args) &;

        StringOutputFormat format(const std::string_view entryVTBegin) const &;
    protected:
        std::atomic<bool> enabled{};
    private:
        std::optional<std::source_location> location;
    };
}

export namespace Glas
{
    LocationField::LocationField(const LocationField& other) :
        PaddingProperty{ other },
        VTSequence{ other }
    {
        enabled.store(other.enabled.load(std::memory_order_relaxed), std::memory_order_relaxed);
    }

    void LocationField::enable() & noexcept {
        enabled.store(true, std::memory_order_relaxed);
    }

    void LocationField::disable() & noexcept {
        enabled.store(false, std::memory_order_relaxed);
    }

    template <typename... Ts>
        requires LocationFieldPrepareSupport<Ts...>
    void LocationField::prepare(Ts&&... args) & {
        const auto unpacker = [&]<typename T>(T&& arg) {
            if constexpr (std::same_as<std::remove_cvref_t<T>, LocationPrepare>) {
                location.emplace(arg.location);
            }
        };

        ((unpacker.operator()<Ts>(std::forward<Ts>(args))), ...);
    }

    StringOutputFormat LocationField::format(const std::string_view entryVTBegin) const & {
        std::string text;

        const auto paddingSnapshot = paddingStrings.load(std::memory_order_relaxed);
        if (paddingSnapshot) {
            text.append(paddingSnapshot->breakBefore);
            text.append(paddingSnapshot->spaceBefore);
        }

        text.append("*Location:*");
        text.append("\n- File: `");

#ifdef NDEBUG 
        const std::string file{ std::filesystem::path(location.value().file_name()).
            filename().string() };
#else
        const std::string file{ location.value().file_name() };
#endif

        text.append(file);
        text.push_back('`');

        text.append("\n- Line: ");
        text.append(std::to_string(location.value().line()));

        text.append("\n- Column: ");
        text.append(std::to_string(location.value().column()));

        text.append("\n- Function: `");
        text.append(location.value().function_name());
        text.push_back('`');

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