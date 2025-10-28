export module Glas.TimePointField;
export import Glas.VTSequence;
export import Glas.StringOutputFormat;
export import Glas.Padding;
export import Glas.Exception;
export import std;


export namespace Glas
{
    class TimePointField :
        public PaddingProperty,
        public VTSequence
    {
    public:
        enum class Precision { None, Milli, Micro, Nano };
    public:
        TimePointField(const TimePointField& other);

        TimePointField() = default;
        virtual ~TimePointField() = default;
    private:
        TimePointField& operator=(const TimePointField&) = delete;
        TimePointField(TimePointField&&) noexcept = delete;
        TimePointField& operator=(TimePointField&&) noexcept = delete;
    public:
        void enable() & noexcept;
        void disable() & noexcept;
        void precision(const Precision precision);
    protected:
        void prepare(auto&&... args) &;

        StringOutputFormat format(const std::string_view entryVTBegin) const &;
    protected:
        std::atomic<bool> enabled{};
    private:
        std::atomic<Precision> timePrecision{ Precision::None };
        std::optional<std::chrono::system_clock::time_point> timePoint;
    };
}

export namespace Glas
{
    TimePointField::TimePointField(const TimePointField& other) : 
        PaddingProperty{ other },
        VTSequence{ other }
    {
        enabled.store(other.enabled.load(std::memory_order_relaxed), std::memory_order_relaxed);

        timePrecision.store(other.timePrecision.load(std::memory_order_relaxed),
            std::memory_order_relaxed);
    }

    void TimePointField::enable() & noexcept {
        enabled.store(true, std::memory_order_relaxed);
    }

    void TimePointField::disable() & noexcept {
        enabled.store(false, std::memory_order_relaxed);
    }

    void TimePointField::precision(const Precision precision) {
        timePrecision.store(precision, std::memory_order_relaxed);
    }

    void TimePointField::prepare(auto&&... args) & {
        timePoint.emplace(std::chrono::system_clock::now());
    }

    StringOutputFormat TimePointField::format(const std::string_view entryVTBegin) const & {
		std::string text;

        const auto paddingSnapshot = paddingStrings.load(std::memory_order_relaxed);
		if (paddingSnapshot) {
			text.append(paddingSnapshot->breakBefore);
			text.append(paddingSnapshot->spaceBefore);
		}

		const std::chrono::zoned_time<std::chrono::seconds> zonedTime{
				std::chrono::current_zone(),
				std::chrono::floor<std::chrono::seconds>(timePoint.value())
		};

		std::format_to(std::back_inserter(text), "{:%d.%m.%Y %H:%M:%S}", zonedTime);

        switch (timePrecision.load(std::memory_order_acquire)) {
        case Precision::Milli:
        {
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                timePoint.value().time_since_epoch()) % 1000;
            std::format_to(std::back_inserter(text), ".{:03}", ms.count());
        }
        break;
        case Precision::Micro:
        {
            auto us = std::chrono::duration_cast<std::chrono::microseconds>(
                timePoint.value().time_since_epoch()) % 1'000'000;
            std::format_to(std::back_inserter(text), ".{:06}", us.count());
        }
        break;
        case Precision::Nano:
        {
            auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                timePoint.value().time_since_epoch()) % 1'000'000'000;
            std::format_to(std::back_inserter(text), ".{:09}", ns.count());
        }
        break;
        }

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