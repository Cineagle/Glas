export module Glas.ThreadIDField;
export import Glas.VTSequence;
export import Glas.StringOutputFormat;
export import Glas.Padding;
export import Glas.Exception;
export import std;


export namespace Glas
{
    class ThreadIDField :
        public PaddingProperty,
        public VTSequence
    {
    public:
        ThreadIDField(const ThreadIDField& other);

        ThreadIDField() = default;
        virtual ~ThreadIDField() = default;
    private:
        ThreadIDField& operator=(const ThreadIDField&) = delete;
        ThreadIDField(ThreadIDField&&) noexcept = delete;
        ThreadIDField& operator=(ThreadIDField&&) noexcept = delete;
    public:
        void enable() & noexcept;
        void disable() & noexcept;
    protected:
        void prepare(auto&&... args) &;

        StringOutputFormat format(const std::string_view entryVTBegin) const &;
    protected:
        std::atomic<bool> enabled{};
    private:
        std::optional<std::thread::id> threadID;
    };
}

export namespace Glas
{
    ThreadIDField::ThreadIDField(const ThreadIDField& other) :
        PaddingProperty{ other },
        VTSequence{ other }
    {
        enabled.store(other.enabled.load(std::memory_order_relaxed), std::memory_order_relaxed);
    }

    void ThreadIDField::enable() & noexcept {
        enabled.store(true, std::memory_order_relaxed);
    }

    void ThreadIDField::disable() & noexcept {
        enabled.store(false, std::memory_order_relaxed);
    }

    void ThreadIDField::prepare(auto&&... args) & {
        threadID.emplace(std::this_thread::get_id());
    }

    StringOutputFormat ThreadIDField::format(const std::string_view entryVTBegin) const & {
        std::string text;

        const auto paddingSnapshot = paddingStrings.load(std::memory_order_relaxed);
        if (paddingSnapshot) {
            text.append(paddingSnapshot->breakBefore);
            text.append(paddingSnapshot->spaceBefore);
        }

        std::format_to(std::back_inserter(text), "[{}]", threadID.value());

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