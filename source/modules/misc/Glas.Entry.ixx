export module Glas.Entry;
export import std;


export namespace Glas
{
    class Entry {
    public:
        enum class Scheme { Queue, Direct };
    public:
        Entry(const Entry& other);

        Entry() = default;
        virtual ~Entry() = default;
    private:
        Entry& operator=(const Entry&) = delete;
        Entry(Entry&&) noexcept = delete;
        Entry& operator=(Entry&&) noexcept = delete;
    public:
        void scheme(const Scheme scheme);
    public:
        virtual void expose() & = 0;
    protected:
        std::atomic<Scheme> outputScheme{ Scheme::Queue };
    };
}

export namespace Glas
{
    Entry::Entry(const Entry& other) {
        outputScheme.store(other.outputScheme.load(std::memory_order_relaxed), 
            std::memory_order_relaxed);
    }

    void Entry::scheme(const Scheme scheme) {
        outputScheme.store(scheme, std::memory_order_relaxed);
    }
}