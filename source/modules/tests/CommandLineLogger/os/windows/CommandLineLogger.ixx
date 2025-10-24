export module Glas.Test.CommandLineLogger;
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
	class CommandLineLogger :
		public
			Logger<
				InfoEntry<
					TimePointField, MessageField
				>,
				WarningEntry<
					TimePointField, MessageField
				>,
				ErrorEntry<
					TimePointField, MessageField, ErrorCodeField
				>
			>
	{
	private:
		CommandLineLogger(StringLike auto&& loggerName,
			const Queue::Scheme queueScheme,
			const std::size_t queueCapacity);
	public:
		static std::shared_ptr<CommandLineLogger> create(StringLike auto&& loggerName,
			const Queue::Scheme queueScheme = Queue::Scheme::Bound,
			const std::size_t queueCapacity = 1024);
	private:
		void createOutputs() &;
		void tune() &;
		void tuneInfoEntry() &;
		void tuneWarningEntry() &;
		void tuneErrorEntry() &;
	private:
		std::shared_ptr<Output<>> console;
		std::shared_ptr<Output<>> file;
		std::shared_ptr<Output<>> debug;
	};
}

export namespace Glas::Test
{
	CommandLineLogger::CommandLineLogger(StringLike auto&& loggerName,
		const Queue::Scheme queueScheme, const std::size_t queueCapacity) :

		Logger<
			InfoEntry<
				TimePointField, MessageField
			>,
			WarningEntry<
				TimePointField, MessageField
			>,
			ErrorEntry<
				TimePointField, MessageField, ErrorCodeField
			>
		>{ std::forward<decltype(loggerName)>(loggerName), queueScheme, queueCapacity }
	{
		createOutputs();
		tune();
	}

	std::shared_ptr<CommandLineLogger> CommandLineLogger::create(
		StringLike auto&& loggerName, const Queue::Scheme queueScheme,
		const std::size_t queueCapacity)
	{
		auto ptr = std::unique_ptr<CommandLineLogger>{
			new CommandLineLogger(std::forward<decltype(loggerName)>(loggerName), queueScheme,
				queueCapacity)
		};
		return std::shared_ptr<CommandLineLogger>(std::move(ptr));
	}

	void CommandLineLogger::createOutputs() & {
		console = ConsoleStringOutput::create(
			ConsoleConfig{
				.consoleTitle{ L"CommandLineLogger" },
				.fontFaceName{ L"Cascadia Mono" }
			}
		);

		file = FileStringOutput::create("lg1");

		debug = DebugStringOutput::create();
	}

	void CommandLineLogger::tune() & {
		tuneInfoEntry();
		tuneWarningEntry();
		tuneErrorEntry();
	}

	void CommandLineLogger::tuneInfoEntry() & {
		this->InfoEntry::TimePointField::enable();
		this->InfoEntry::MessageField::enable();

		this->InfoEntry::TimePointField::padding(Padding{ .spaceAfter{ 2 } });
		this->InfoEntry::MessageField::padding(Padding{ .breakAfter{ 1 } });

		this->InfoEntry::outputs(file, console);
	}

	void CommandLineLogger::tuneWarningEntry() & {
		this->WarningEntry::TimePointField::enable();
		this->WarningEntry::MessageField::enable();

		this->WarningEntry::TimePointField::padding(Padding{ .spaceAfter{ 2 } });
		this->WarningEntry::MessageField::padding(Padding{ .breakAfter{ 1 } });

		this->WarningEntry::outputs(file, console);
	}

	void CommandLineLogger::tuneErrorEntry() & {
		this->ErrorEntry::TimePointField::enable();
		this->ErrorEntry::MessageField::enable();
		this->ErrorEntry::ErrorCodeField::enable();

		this->ErrorEntry::TimePointField::padding(Padding{ .spaceAfter{ 2 } });
		this->ErrorEntry::MessageField::padding(Padding{ .breakAfter{ 1 } });
		this->ErrorEntry::ErrorCodeField::padding(Padding{ .breakAfter{ 1 } });

		this->ErrorEntry::outputs(debug, file, console);
	}
}