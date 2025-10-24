module;
#include <windows.h>

export module Glas.ErrorCodeField;
export import Glas.VTSequence;
export import Glas.StringOutputFormat;
export import Glas.Padding;
export import Glas.Exception;
export import std;


export namespace Glas
{
    class ErrorCodeField :
        public PaddingProperty,
        public VTSequence
    {
    public:
        ErrorCodeField(const ErrorCodeField& other);

        ErrorCodeField() = default;
        virtual ~ErrorCodeField() = default;
    private:
        ErrorCodeField& operator=(const ErrorCodeField&) = delete;
        ErrorCodeField(ErrorCodeField&&) noexcept = delete;
        ErrorCodeField& operator=(ErrorCodeField&&) noexcept = delete;
    public:
        void enable() & noexcept;
        void disable() & noexcept;
    protected:
        void prepare(auto&&... args) &;
        StringOutputFormat format(const std::string_view entryVTBegin) const &;
    protected:
        std::atomic<bool> enabled{};
    private:
        std::optional<std::error_code> errorCode;
    };
}

export namespace Glas
{
    ErrorCodeField::ErrorCodeField(const ErrorCodeField& other) :
        PaddingProperty{ other },
        VTSequence{ other }
    {
        enabled.store(other.enabled.load(std::memory_order_relaxed), std::memory_order_relaxed);
    }

    void ErrorCodeField::enable() & noexcept {
        enabled.store(true, std::memory_order_relaxed);
    }

    void ErrorCodeField::disable() & noexcept {
        enabled.store(false, std::memory_order_relaxed);
    }

    void ErrorCodeField::prepare(auto&&... args) & {
        errorCode.emplace(static_cast<int>(GetLastError()), std::system_category());
    }

    StringOutputFormat ErrorCodeField::format(const std::string_view entryVTBegin) const & {
        std::string text;

        const auto paddingSnapshot = std::atomic_load_explicit(&paddingStrings,
            std::memory_order_acquire);

        if (paddingSnapshot) {
            text.append(paddingSnapshot->breakBefore);
            text.append(paddingSnapshot->spaceBefore);
        }

        text.append("*Last Error Code:*");

        text.append("\n- Value: ");
        text.append(std::to_string(errorCode.value().value()));

        text.append("\n- Description: `");
        text.append(errorCode.value().message());
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