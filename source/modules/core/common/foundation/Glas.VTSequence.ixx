export module Glas.VTSequence;
export import Glas.VTStyle;
export import std;


export namespace Glas
{
    class VTSequence {
    public:
        VTSequence(const VTSequence& other);
        VTSequence(VTSequence&& other) noexcept;

        VTSequence() = default;
        virtual ~VTSequence() = default;
    private:
        VTSequence& operator=(const VTSequence&) = delete;
        VTSequence& operator=(VTSequence&&) noexcept = delete;
    public:
        static std::string formVTBegin(const VTStyle style);
    public:
        void enable() & noexcept;
        void disable() & noexcept;
        void style(const VTStyle style) &;
    protected:
        std::atomic<bool> enabled{};
        std::atomic<std::shared_ptr<std::string>> vtBegin;
    };
}

export namespace Glas
{
    VTSequence::VTSequence(const VTSequence& other) {
        enabled.store(other.enabled.load(std::memory_order_relaxed), std::memory_order_relaxed);

        const auto snapshot = other.vtBegin.load(std::memory_order_relaxed);
        if (snapshot) {
            vtBegin.store(std::make_shared<std::string>(*snapshot),
                std::memory_order_relaxed);
        }
    }

    VTSequence::VTSequence(VTSequence&& other) noexcept {
        enabled.store(other.enabled.load(std::memory_order_relaxed), std::memory_order_relaxed);

        const auto snapshot = other.vtBegin.load(std::memory_order_relaxed);
        if (snapshot) {
            vtBegin.store(std::make_shared<std::string>(std::move(*snapshot)),
                std::memory_order_relaxed);
        }
    }

    void VTSequence::enable() & noexcept {
        enabled.store(true, std::memory_order_relaxed);
    }

    void VTSequence::disable() & noexcept {
        enabled.store(false, std::memory_order_relaxed);
    }

    void VTSequence::style(const VTStyle style) & {
        vtBegin.store(std::make_shared<std::string>(formVTBegin(style)),
            std::memory_order_relaxed);
    }

    std::string VTSequence::formVTBegin(const VTStyle style) {
        std::string text{ "\x1b[" };

        auto first{ true };

        if (style.effect != VTStyle::Effect::None) {
            text.append(std::to_string(static_cast<int>(style.effect)));
            first = false;
        }

        if (!first) {
            text.push_back(';');
        }
        text.append("38;2;");
        text.append(std::to_string(style.fgColor.red));
        text.push_back(';');
        text.append(std::to_string(style.fgColor.green));
        text.push_back(';');
        text.append(std::to_string(style.fgColor.blue));
        first = false;

        if (!first) {
            text.push_back(';');
        }
        text.append("48;2;");
        text.append(std::to_string(style.bgColor.red));
        text.push_back(';');
        text.append(std::to_string(style.bgColor.green));
        text.push_back(';');
        text.append(std::to_string(style.bgColor.blue));

        text.push_back('m');

        return text;
    }
}