export module Glas.Test.CustomEntryLogger;
export import Glas.Logger;

export import Glas.InfoEntry;
export import Glas.ErrorEntry;
export import Glas.Test.CriticalEntry;

export import Glas.TimePointField;
export import Glas.MessageField;
export import Glas.ErrorCodeField;

export import Glas.FileStringOutput;
export import Glas.ConsoleStringOutput;
export import Glas.DebugStringOutput;

export import std;


export namespace Glas::Test
{
	class CustomEntryLogger :
		public
			Logger<
				InfoEntry<
					TimePointField, MessageField
				>,
				ErrorEntry<
					TimePointField, MessageField, ErrorCodeField
				>,
				CriticalEntry<
					TimePointField, MessageField
				>
			>
	{
	private:
		CustomEntryLogger(StringLike auto&& loggerName,
			const Queue::Scheme queueScheme,
			const std::size_t queueCapacity);
	public:
		static std::shared_ptr<CustomEntryLogger> create(StringLike auto&& loggerName,
			const Queue::Scheme queueScheme = Queue::Scheme::Bound,
			const std::size_t queueCapacity = 1024);
	private:
		void createOutputs() &;
		void tune() &;
		void tuneInfoEntry() &;
		void tuneErrorEntry() &;
		void tuneCriticalEntry() &;
	private:
		std::shared_ptr<Output<>> console;
		std::shared_ptr<Output<>> file;
		std::shared_ptr<Output<>> debug;
	};
}

export namespace Glas::Test
{
	CustomEntryLogger::CustomEntryLogger(StringLike auto&& loggerName,
		const Queue::Scheme queueScheme, const std::size_t queueCapacity) :

		Logger<
			InfoEntry<
				TimePointField, MessageField
			>,
			ErrorEntry<
				TimePointField, MessageField, ErrorCodeField
			>,
			CriticalEntry<
				TimePointField, MessageField
			>
		>{ std::forward<decltype(loggerName)>(loggerName), queueScheme, queueCapacity }
	{
		createOutputs();
		tune();
	}

	std::shared_ptr<CustomEntryLogger> CustomEntryLogger::create(
		StringLike auto&& loggerName, const Queue::Scheme queueScheme,
		const std::size_t queueCapacity)
	{
		auto ptr = std::unique_ptr<CustomEntryLogger>{
			new CustomEntryLogger(std::forward<decltype(loggerName)>(loggerName), queueScheme,
				queueCapacity)
		};
		return std::shared_ptr<CustomEntryLogger>(std::move(ptr));
	}

	void CustomEntryLogger::createOutputs() & {
		console = ConsoleStringOutput::create(
			ConsoleConfig{
				.consoleTitle{ L"CustomEntryLogger" },
				.fontFaceName{ L"Cascadia Mono" },
				.vtEnabled{ true }
			}
		);

		file = FileStringOutput::create("CustomEntryLogger.txt",
			FileStringOutput::PathType::Relative);

		debug = DebugStringOutput::create();
	}

	void CustomEntryLogger::tune() & {
		tuneInfoEntry();
		tuneErrorEntry();
		tuneCriticalEntry();
	}

	void CustomEntryLogger::tuneInfoEntry() & {
		this->InfoEntry::TimePointField::enable();
		this->InfoEntry::MessageField::enable();

		this->InfoEntry::TimePointField::padding(Padding{ .spaceAfter{ 2 } });
		this->InfoEntry::MessageField::padding(Padding{ .breakAfter{ 1 } });

		this->InfoEntry::outputs(file, console);
	}

	void CustomEntryLogger::tuneErrorEntry() & {
		this->ErrorEntry::TimePointField::enable();
		this->ErrorEntry::MessageField::enable();
		this->ErrorEntry::ErrorCodeField::enable();

		this->ErrorEntry::TimePointField::padding(Padding{ .spaceAfter{ 2 } });
		this->ErrorEntry::MessageField::padding(Padding{ .breakAfter{ 1 } });
		this->ErrorEntry::ErrorCodeField::padding(Padding{ .breakAfter{ 1 } });

		this->ErrorEntry::outputs(debug, file, console);
	}

	void CustomEntryLogger::tuneCriticalEntry() & {
		this->CriticalEntry::TimePointField::enable();
		this->CriticalEntry::MessageField::enable();

		this->CriticalEntry::TimePointField::padding(Padding{ .spaceAfter{ 2 } });
		this->CriticalEntry::MessageField::padding(Padding{ .breakAfter{ 1 } });

		this->CriticalEntry::TimePointField::VTSequence::enable();
		this->CriticalEntry::MessageField::VTSequence::enable();

		this->CriticalEntry::outputs(file, console);
	}
}