export module Glas.Queue;
export import Glas.Entry;
export import Glas.Exception;
export import std;


export namespace Glas
{
    class Queue final {
    public:
        enum class Scheme { Bound, Unbound };
    private:
        class Slot {
        public:
            std::unique_ptr<Entry> entry;
            alignas(std::hardware_destructive_interference_size)
            std::atomic<bool> ready{};
        };
    public:
        Queue(const Scheme scheme, const std::size_t capacity);
        ~Queue();
    private:
        Queue() = delete;
        Queue(const Queue&) = delete;
        Queue& operator=(const Queue&) = delete;
        Queue(Queue&&) noexcept = delete;
        Queue& operator=(Queue&&) noexcept = delete;
    public:
        std::size_t capacity() const & noexcept;
        std::size_t occupancy() const & noexcept;

        void enqueue(std::unique_ptr<Entry>&& entry) &;
        void dequeue(const std::stop_token token) &;
    private:
        bool blockEnqueue() &;
        void resizeBuffer() &;
    public:
        const Scheme scheme{ Scheme::Bound };

        std::size_t bufferCapacity{};
        std::size_t mask{};
        std::vector<Slot> buffer;

        alignas(std::hardware_destructive_interference_size)
        std::atomic<std::size_t> queued{};

        alignas(std::hardware_destructive_interference_size)
        std::atomic<std::size_t> toDequeue{};

        alignas(std::hardware_destructive_interference_size)
        std::atomic<std::size_t> head{};

        std::size_t tail{};

        alignas(std::hardware_destructive_interference_size)
        std::atomic<bool> enqueueWake{};

        alignas(std::hardware_destructive_interference_size)
        std::atomic<bool> dequeueWake{};

        alignas(std::hardware_destructive_interference_size)
        std::atomic<std::size_t> peakOccupancy{};

        alignas(std::hardware_destructive_interference_size)
        std::atomic<bool> blockStageOne{};

        alignas(std::hardware_destructive_interference_size)
        std::atomic<std::size_t> inEnqueueStageOne{};

        alignas(std::hardware_destructive_interference_size)
        std::atomic<bool> blockStageTwo{};

        alignas(std::hardware_destructive_interference_size)
        std::atomic<std::size_t> inEnqueueStageTwo{};

        std::jthread consumer;
        std::stop_token token;
    };

}

export namespace Glas
{
    Queue::Queue(const Scheme scheme, const std::size_t capacity) :
        scheme{ scheme },
        bufferCapacity{ std::bit_ceil(capacity) },
        mask{ std::bit_ceil(capacity) - 1 },
        buffer{ std::bit_ceil(capacity) }
    {
        if (!capacity) {
            throw Exception{ "queue capacity must be > 0." };
        }

        consumer = std::jthread{ std::bind_front(&Queue::dequeue, this) };
        token = consumer.get_stop_token();
    }

    Queue::~Queue() {
        consumer.request_stop();

        dequeueWake.store(true, std::memory_order_release);
        dequeueWake.notify_one();

        if (consumer.joinable()) {
            consumer.join();
        }
    }

    std::size_t Queue::capacity() const & noexcept {
        return bufferCapacity;
    }

    std::size_t Queue::occupancy() const & noexcept {
        return peakOccupancy.load(std::memory_order_acquire);
    }

    void Queue::enqueue(std::unique_ptr<Entry>&& entry) & {
        if (scheme == Scheme::Unbound) {
            inEnqueueStageOne.fetch_add(1, std::memory_order_acq_rel);
            blockStageOne.wait(true, std::memory_order_acquire);

            inEnqueueStageTwo.fetch_add(1, std::memory_order_acq_rel);
            blockStageTwo.wait(true, std::memory_order_acquire);
        }

        while (true) {
            auto count = queued.load(std::memory_order_acquire);
            if (count < bufferCapacity) {
                if (queued.compare_exchange_weak(count, count + 1, std::memory_order_acq_rel,
                    std::memory_order_acquire))
                {

                    const auto current{ count + 1 };
                    auto prev = peakOccupancy.load(std::memory_order_relaxed);
                    while (current > prev &&
                        !peakOccupancy.compare_exchange_weak(prev, current,
                            std::memory_order_relaxed))
                    {}

                    break;
                }
            }
            else {
                enqueueWake.wait(false, std::memory_order_relaxed);
                enqueueWake.store(false, std::memory_order_release);
            }
        }

        const auto pos = head.fetch_add(1, std::memory_order_acq_rel) & mask;
        buffer[pos].entry = std::move(entry);
        buffer[pos].ready.store(true, std::memory_order_release);

        toDequeue.fetch_add(1, std::memory_order_acq_rel);
        dequeueWake.store(true, std::memory_order_release);
        dequeueWake.notify_one();

        if (scheme == Scheme::Unbound) {
            inEnqueueStageOne.fetch_sub(1, std::memory_order_acq_rel);
            inEnqueueStageTwo.fetch_sub(1, std::memory_order_acq_rel);
        }
    }

    void Queue::dequeue(const std::stop_token token) & {
        while (true) {
            if (scheme == Scheme::Unbound &&
                peakOccupancy.load(std::memory_order_relaxed) == bufferCapacity &&
                blockEnqueue())
            {
                resizeBuffer();
            }

            if (toDequeue.load(std::memory_order_acquire)) {
                if (buffer[tail].ready.load(std::memory_order_acquire)) {
                    buffer[tail].entry->expose();
                    buffer[tail].ready.store(false, std::memory_order_release);
                    ++tail &= mask;

                    toDequeue.fetch_sub(1, std::memory_order_acq_rel);
                    queued.fetch_sub(1, std::memory_order_acq_rel);

                    enqueueWake.store(true, std::memory_order_release);
                    enqueueWake.notify_all();
                }
            }
            else {
                if (token.stop_requested() &&
                    !toDequeue.load(std::memory_order_acquire))
                {
                    break;
                }

                if (scheme == Scheme::Unbound &&
                    peakOccupancy.load(std::memory_order_relaxed) == bufferCapacity)
                {
                    if (blockEnqueue()) {
                        resizeBuffer();
                    }
                    continue;
                }

                dequeueWake.wait(false, std::memory_order_acquire);
                dequeueWake.store(false, std::memory_order_release);
            }
        }
    }

    bool Queue::blockEnqueue() & {
        if (blockStageOne.load(std::memory_order_acquire)) {
            if (inEnqueueStageTwo.load(std::memory_order_acquire)) {
                blockStageTwo.store(false, std::memory_order_release);
                blockStageTwo.notify_all();
                return false;
            }
            return true;
        }
        else {
            blockStageOne.store(true, std::memory_order_release);
            blockStageTwo.store(true, std::memory_order_release);
            if (inEnqueueStageOne.load(std::memory_order_acquire)) {
                return false;
            }
            return true;
        }
    }

    void Queue::resizeBuffer() & {
        const auto size = bufferCapacity * 2;
        std::vector<Slot> newBuffer(size);

        const auto count = (head.load(std::memory_order_relaxed) - tail) & mask;
        
        for (std::size_t i{}; i < count; ++i)  {
            const auto idx = (tail + i) & mask;

            newBuffer[i].entry = std::move(buffer[idx].entry);
            newBuffer[i].ready.store(buffer[idx].ready.load(std::memory_order_relaxed),
                std::memory_order_relaxed);
        }

        buffer = std::move(newBuffer);
        bufferCapacity = size;
        mask = size - 1;
        tail = 0;
        head = count;

        blockStageOne.store(false, std::memory_order_release);
        blockStageTwo.store(false, std::memory_order_release);
        blockStageOne.notify_all();
        blockStageTwo.notify_all();
    }
}