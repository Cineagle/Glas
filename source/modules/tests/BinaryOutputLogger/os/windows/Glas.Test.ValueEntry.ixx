export module Glas.Test.ValueEntry;
export import Glas.Entry;
export import Glas.OutputManager;
export import Glas.Exception;
export import std;


export namespace Glas::Test
{
    class ValueEntry :
        public Entry,
        public OutputManager<std::vector<std::byte>>
    {
    public:
        ValueEntry() = default;

        ValueEntry(const ValueEntry&) = default;
        virtual ~ValueEntry() override = default;
    private:
        ValueEntry& operator=(const ValueEntry&) = delete;
        ValueEntry(ValueEntry&&) noexcept = delete;
        ValueEntry& operator=(ValueEntry&&) noexcept = delete;
    public:
        template <typename T>
        void value(this auto& self, const void* address);
    private:
        virtual void expose() & override;
    private:
        template <typename T>
        auto prepare(this auto& self, const void* address);
        void deliver(this auto& self, std::unique_ptr<ValueEntry>&& entry);
        void output() const &;
    private:
        std::vector<std::byte> bytes;
    };
}

export namespace Glas::Test
{
    template <typename T>
    void ValueEntry::value(this auto& self, const void* address) {
        self.ValueEntry::deliver(
            self.ValueEntry::prepare<T>(address)
        );
    }

    template <typename T>
    auto ValueEntry::prepare(this auto& self, const void* address) {
        if (!address) {
            throw Exception{ "`address` is nullptr." };
        }

        auto entry = std::make_unique<ValueEntry>(self);
        entry->bytes.resize(sizeof(T));
        std::memcpy(entry->bytes.data(), address, sizeof(T));

        return entry;
    }

    void ValueEntry::deliver(this auto& self, std::unique_ptr<ValueEntry>&& entry) {
        if (entry->Entry::outputScheme.load(std::memory_order_relaxed) == Scheme::Queue) {
            self.enqueue(std::move(entry));
        }
        else {
            entry->expose();
        }
    }

    void ValueEntry::expose() & {
        output();
    }

    void ValueEntry::output() const & {
        const auto outputs = this->OutputManager::sharedOutputs.load(std::memory_order_relaxed);
        if (outputs) {
            for (const auto& output : *outputs) {
                output->output(bytes);
            }
        }
    }
}