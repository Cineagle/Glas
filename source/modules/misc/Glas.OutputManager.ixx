export module Glas.OutputManager;
export import Glas.Output;
export import Glas.StringOutputFormat;
export import Glas.Concepts;
export import std;


export namespace Glas
{
    template <typename T = std::vector<StringOutputFormat>>
    class OutputManager {
    public:
        using SharedOutput = std::shared_ptr<Output<T>>;
        using SharedOutputsVector = std::vector<SharedOutput>;
    public:
        OutputManager(const OutputManager& other);

        OutputManager() = default;
        virtual ~OutputManager() = default;
    private:
        OutputManager& operator=(const OutputManager&) = delete;
        OutputManager(OutputManager&&) noexcept = delete;
        OutputManager& operator=(OutputManager&&) noexcept = delete;
    public:
        bool outputs(std::convertible_to<SharedOutput> auto&&... outputs) &;
        bool outputs(const SharedOutputsVector& outputs) &;
        bool outputs(SharedOutputsVector&& outputs) &;
        void clear() &;
    protected:
        void output(ForwardedAs<T> auto&& formatted) const &;
    protected:
        std::atomic<std::shared_ptr<SharedOutputsVector>> sharedOutputs;
    };
}

export namespace Glas
{
    template <typename T>
    OutputManager<T>::OutputManager(const OutputManager& other) {
        const auto snapshot = other.sharedOutputs.load(std::memory_order_relaxed);
        if (snapshot) {
            sharedOutputs.store(std::make_shared<SharedOutputsVector>(*snapshot),
                std::memory_order_relaxed);
        }
    }

    template <typename T>
    bool OutputManager<T>::outputs(std::convertible_to<SharedOutput> auto&&... outputs) & {
        if constexpr (sizeof...(outputs)) {
            SharedOutputsVector unpacked;
            (unpacked.push_back(std::forward<decltype(outputs)>(outputs)), ...);

            const auto ptr = std::make_shared<SharedOutputsVector>(std::move(unpacked));
            sharedOutputs.store(ptr, std::memory_order_relaxed);
            return true;
        }
        return false;
    }

    template <typename T>
    bool OutputManager<T>::outputs(const SharedOutputsVector& outputs) & {
        if (outputs.empty()) {
            return false;
        }
        const auto ptr = std::make_shared<SharedOutputsVector>(outputs);
        sharedOutputs.store(ptr, std::memory_order_relaxed);
        return true;
    }

    template <typename T>
    bool OutputManager<T>::outputs(SharedOutputsVector&& outputs) & {
        if (outputs.empty()) {
            return false;
        }
        const auto ptr = std::make_shared<SharedOutputsVector>(std::move(outputs));
        sharedOutputs.store(ptr, std::memory_order_relaxed);
        return true;
    }

    template <typename T>
    void OutputManager<T>::clear() & {
        sharedOutputs.store(nullptr, std::memory_order_relaxed);
    }

    template <typename T>
    void OutputManager<T>::output(ForwardedAs<T> auto&& formatted) const & {
        const auto outputs = sharedOutputs.load(std::memory_order_relaxed);
        if (outputs) {
            for (const auto& output : *outputs) {
                output->output(formatted);
            }
        }
    }
}