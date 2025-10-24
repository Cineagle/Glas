export module Glas.Logger;
export import Glas.Entry;
export import Glas.Queue;
export import Glas.Concepts;
export import Glas.Exception;
export import std;


export namespace Glas
{
	template <typename T>
	concept LoggerMixins =
		std::is_class_v<T> &&
		std::default_initializable<T> &&
		std::is_copy_constructible_v<T> &&
		std::derived_from<T, Entry>;


	template <LoggerMixins... Mixins>
	class Logger :
		public Mixins... 
	{
	public:
		using Scheme = Queue::Scheme;
	public:
		virtual ~Logger() = default;
	protected:
		Logger(StringLike auto&& loggerName, const Scheme queueScheme,
			const std::size_t queueCapacity);
	private:
		Logger() = delete;
		Logger(const Logger&) = delete;
		Logger& operator=(const Logger&) = delete;
		Logger(Logger&&) noexcept = delete;
		Logger& operator=(Logger&&) noexcept = delete;
	public:
		static std::shared_ptr<Logger<Mixins...>> create(StringLike auto&& loggerName,
			const Scheme queueScheme = Scheme::Bound, const std::size_t queueCapacity = 1024);
	public:
		std::size_t capacity() const & noexcept;
		std::size_t occupancy() const & noexcept;

		void enqueue(std::unique_ptr<Entry>&& entry) &;
	public:
		const std::string loggerName;
	private:
		Queue queue;
	};
}

export namespace Glas
{
	template <LoggerMixins... Mixins>
	Logger<Mixins...>::Logger(StringLike auto&& loggerName,
		const Scheme queueScheme, const std::size_t queueCapacity) :

		loggerName{ std::forward<decltype(loggerName)>(loggerName) },
		queue{ queueScheme, queueCapacity }
	{}

	template <LoggerMixins... Mixins>
	std::shared_ptr<Logger<Mixins...>> Logger<Mixins...>::create(
		StringLike auto&& loggerName, const Scheme queueScheme,
		const std::size_t queueCapacity)
	{
		auto ptr = std::unique_ptr<Logger<Mixins...>>{
			new Logger<Mixins...>(std::forward<decltype(loggerName)>(loggerName), queueScheme,
				queueCapacity)
		};
		return std::shared_ptr<Logger<Mixins...>>(std::move(ptr));
	}

	template <LoggerMixins... Mixins>
	std::size_t Logger<Mixins...>::capacity() const & noexcept {
		return queue.capacity();
	}

	template <LoggerMixins... Mixins>
	std::size_t Logger<Mixins...>::occupancy() const & noexcept {
		return queue.occupancy();
	}

	template <LoggerMixins... Mixins>
	void Logger<Mixins...>::enqueue(std::unique_ptr<Entry>&& entry) & {
		if (!entry) {
			throw Exception{ "`entry` is empty." };
		}
		queue.enqueue(std::move(entry));
	}
}