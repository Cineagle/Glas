export module Glas.Padding;
export import std;


export namespace Glas
{
    class Padding {
    public:
        std::size_t breakBefore{};
        std::size_t spaceBefore{};
        std::size_t spaceAfter{};
        std::size_t breakAfter{};
    };


    class PaddingStrings {
    public:
        explicit PaddingStrings(const Padding padding);
    public:
        std::string breakBefore;
        std::string spaceBefore;
        std::string spaceAfter;
        std::string breakAfter;
    };


    class PaddingProperty {
    public:
        PaddingProperty(const PaddingProperty& other);

        PaddingProperty() = default;
        virtual ~PaddingProperty() = default;
    private:
        PaddingProperty& operator=(const PaddingProperty&) = delete;
        PaddingProperty(PaddingProperty&&) noexcept = delete;
        PaddingProperty& operator=(PaddingProperty&&) noexcept = delete;
    public:
        void padding(const Padding padding) &;
    protected:
        std::atomic<std::shared_ptr<PaddingStrings>> paddingStrings;
    };
}

export namespace Glas
{
    PaddingStrings::PaddingStrings(const Padding padding) :
        breakBefore(padding.breakBefore, '\n'),
        spaceBefore(padding.spaceBefore, ' '),
        spaceAfter(padding.spaceAfter, ' '),
        breakAfter(padding.breakAfter, '\n')
    {}
}

export namespace Glas
{
    PaddingProperty::PaddingProperty(const PaddingProperty& other) {
        const auto snapshot = other.paddingStrings.load(std::memory_order_relaxed);
        if (snapshot) {
            paddingStrings.store(std::make_shared<PaddingStrings>(*snapshot),
                std::memory_order_relaxed);
        }
    }

    void PaddingProperty::padding(const Padding padding) & {
        const auto ptr = std::make_shared<PaddingStrings>(padding);
        paddingStrings.store(ptr, std::memory_order_relaxed);
    }
}