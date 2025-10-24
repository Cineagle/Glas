module;
#include <windows.h>
#include <Lmcons.h>

export module Glas.Test.UserNameField;
export import Glas.VTSequence;
export import Glas.StringOutputFormat;
export import Glas.Padding;
export import Glas.Exception;
export import std;


export namespace Glas::Test
{
    class UserNameField :
        public Glas::PaddingProperty,
        public Glas::VTSequence
    {
    public:
        UserNameField(const UserNameField& other);

        UserNameField() = default;
        virtual ~UserNameField() = default;
    private:
        UserNameField& operator=(const UserNameField&) = delete;
        UserNameField(UserNameField&&) noexcept = delete;
        UserNameField& operator=(UserNameField&&) noexcept = delete;
    public:
        void enable() & noexcept;
        void disable() & noexcept;
    protected:
        void prepare(auto&&... args) &;

        StringOutputFormat format(const std::string_view entryVTBegin) const &;
    protected:
        std::atomic<bool> enabled{};
    private:
        std::optional<std::string> userName;
    };
}

export namespace Glas::Test
{
    UserNameField::UserNameField(const UserNameField& other) :
        Glas::PaddingProperty{ other },
        Glas::VTSequence{ other }
    {
        enabled.store(other.enabled.load(std::memory_order_relaxed), std::memory_order_relaxed);
    }

    void UserNameField::enable() & noexcept {
        enabled.store(true, std::memory_order_relaxed);
    }

    void UserNameField::disable() & noexcept {
        enabled.store(false, std::memory_order_relaxed);
    }

    void UserNameField::prepare(auto&&... args) & {
        char username[UNLEN + 1]{};
        unsigned long size{ UNLEN + 1 };
        if (GetUserNameA(username, &size)) {
            userName.emplace(username);
        }
        else {
            userName.emplace("unknown");
        }
    }

    StringOutputFormat UserNameField::format(const std::string_view entryVTBegin) const & {
        std::string text;

        const auto paddingSnapshot = std::atomic_load_explicit(&paddingStrings,
            std::memory_order_acquire);

        if (paddingSnapshot) {
            text.append(paddingSnapshot->breakBefore);
            text.append(paddingSnapshot->spaceBefore);
        }

        text.push_back('[');
        text.append(userName.value());
        text.push_back(']');

        if (paddingSnapshot) {
            text.append(paddingSnapshot->spaceAfter);
            text.append(paddingSnapshot->breakAfter);
        }

        Glas::StringOutputFormat formatted;

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