export module Glas.OutputManager;
export import Glas.Output;
export import Glas.StringOutputFormat;
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
        std::atomic<std::shared_ptr<SharedOutputsVector>> sharedOutputs;
    };
}

export namespace Glas
{
    template <typename T>
    OutputManager<T>::OutputManager(const OutputManager& other) {
        const auto snapshot = std::atomic_load_explicit(&other.sharedOutputs,
            std::memory_order_relaxed);

        if (snapshot) {
            std::atomic_store_explicit(&sharedOutputs, 
                std::make_shared<SharedOutputsVector>(*snapshot),
                std::memory_order_relaxed);
        }
    }

    template <typename T>
    bool OutputManager<T>::outputs(std::convertible_to<SharedOutput> auto&&... outputs) & {
        if constexpr (sizeof...(outputs)) {
            SharedOutputsVector unpacked;
            (unpacked.push_back(std::forward<decltype(outputs)>(outputs)), ...);

            const auto ptr = std::make_shared<SharedOutputsVector>(std::move(unpacked));
            std::atomic_store_explicit(&sharedOutputs, ptr, std::memory_order_relaxed);
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
        std::atomic_store_explicit(&sharedOutputs, ptr, std::memory_order_relaxed);
        return true;
    }

    template <typename T>
    bool OutputManager<T>::outputs(SharedOutputsVector&& outputs) & {
        if (outputs.empty()) {
            return false;
        }
        const auto ptr = std::make_shared<SharedOutputsVector>(std::move(outputs));
        std::atomic_store_explicit(&sharedOutputs, ptr, std::memory_order_relaxed);
        return true;
    }

    template <typename T>
    void OutputManager<T>::clear() & {
        std::atomic_store_explicit(&sharedOutputs, nullptr, std::memory_order_relaxed);
    }
}