export module Glas.Test.UnboundQueueLogger;
export import Glas.Logger;

export import Glas.InfoEntry;
export import Glas.WarningEntry;
export import Glas.ErrorEntry;

export import Glas.TimePointField;
export import Glas.MessageField;
export import Glas.ErrorCodeField;

export import Glas.FileStringOutput;
export import Glas.ConsoleStringOutput;
export import Glas.DebugStringOutput;

export import std;


export namespace Glas::Test
{
	class UnboundQueueLogger :
		public
			Glas::Logger<
				Glas::InfoEntry<
					Glas::TimePointField, Glas::MessageField
				>,
				Glas::WarningEntry<
					Glas::TimePointField, Glas::MessageField
				>,
				Glas::ErrorEntry<
					Glas::TimePointField, Glas::MessageField, Glas::ErrorCodeField
				>
			>
	{
	public:
		using Scheme = Glas::Queue::Scheme;
	private:
		UnboundQueueLogger(StringLike auto&& loggerName,
			const Glas::Queue::Scheme queueScheme,
			const std::size_t queueCapacity);
	public:
		static std::shared_ptr<UnboundQueueLogger> create(StringLike auto&& loggerName,
			const Glas::Queue::Scheme queueScheme = Glas::Queue::Scheme::Bound,
			const std::size_t queueCapacity = 1024);
	private:
		void createOutputs() &;
		void tune() &;
		void tuneInfoEntry() &;
		void tuneWarningEntry() &;
		void tuneErrorEntry() &;
	private:
		std::shared_ptr<Glas::Output<>> console;
		std::shared_ptr<Glas::Output<>> file;
		std::shared_ptr<Glas::Output<>> debug;
	};
}

export namespace Glas::Test
{
	UnboundQueueLogger::UnboundQueueLogger(StringLike auto&& loggerName,
		const Glas::Queue::Scheme queueScheme, const std::size_t queueCapacity) :

		Glas::Logger<
			Glas::InfoEntry<
				Glas::TimePointField, Glas::MessageField
			>,
			Glas::WarningEntry<
				Glas::TimePointField, Glas::MessageField
			>,
			Glas::ErrorEntry<
				Glas::TimePointField, Glas::MessageField, Glas::ErrorCodeField
			>
		>{ std::forward<decltype(loggerName)>(loggerName), queueScheme, queueCapacity }
	{
		createOutputs();
		tune();
	}

	std::shared_ptr<UnboundQueueLogger> UnboundQueueLogger::create(
		StringLike auto&& loggerName, const Glas::Queue::Scheme queueScheme,
		const std::size_t queueCapacity)
	{
		auto ptr = std::unique_ptr<UnboundQueueLogger>{
			new UnboundQueueLogger(std::forward<decltype(loggerName)>(loggerName), queueScheme,
				queueCapacity)
		};
		return std::shared_ptr<UnboundQueueLogger>(std::move(ptr));
	}

	void UnboundQueueLogger::createOutputs() & {
		console = Glas::ConsoleStringOutput::create(
			Glas::ConsoleConfig{
				.consoleTitle{ L"UnboundQueueLogger" },
				.fontFaceName{ L"Cascadia Mono" }
			}
		);

		file = Glas::FileStringOutput::create("UnboundQueueLogger.txt",
			Glas::FileStringOutput::PathType::Relative);

		debug = Glas::DebugStringOutput::create();
	}

	void UnboundQueueLogger::tune() & {
		tuneInfoEntry();
		tuneWarningEntry();
		tuneErrorEntry();
	}

	void UnboundQueueLogger::tuneInfoEntry() & {
		this->InfoEntry::TimePointField::enable();
		this->InfoEntry::MessageField::enable();

		this->InfoEntry::TimePointField::padding(Glas::Padding{ .spaceAfter{ 2 } });
		this->InfoEntry::MessageField::padding(Glas::Padding{ .breakAfter{ 1 } });

		this->InfoEntry::outputs(file, console);
	}

	void UnboundQueueLogger::tuneWarningEntry() & {
		this->WarningEntry::TimePointField::enable();
		this->WarningEntry::MessageField::enable();

		this->WarningEntry::TimePointField::padding(Glas::Padding{ .spaceAfter{ 2 } });
		this->WarningEntry::MessageField::padding(Glas::Padding{ .breakAfter{ 1 } });

		this->WarningEntry::outputs(file, console);
	}

	void UnboundQueueLogger::tuneErrorEntry() & {
		this->ErrorEntry::TimePointField::enable();
		this->ErrorEntry::MessageField::enable();
		this->ErrorEntry::ErrorCodeField::enable();

		this->ErrorEntry::TimePointField::padding(Glas::Padding{ .spaceAfter{ 2 } });
		this->ErrorEntry::MessageField::padding(Glas::Padding{ .breakAfter{ 1 } });
		this->ErrorEntry::ErrorCodeField::padding(Glas::Padding{ .breakAfter{ 1 } });

		this->ErrorEntry::outputs(debug, file, console);
	}
}