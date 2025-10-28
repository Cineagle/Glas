export module Glas.Test.CustomStringOutputLogger;
export import Glas.Logger;

export import Glas.InfoEntry;
export import Glas.ErrorEntry;

export import Glas.TimePointField;
export import Glas.MessageField;
export import Glas.ErrorCodeField;

export import Glas.FileStringOutput;
export import Glas.ConsoleStringOutput;
export import Glas.DebugStringOutput;
export import Glas.Test.WindowsEventOutput;

export import std;


export namespace Glas::Test
{
	class CustomStringOutputLogger :
		public
			Logger<
				InfoEntry<
					TimePointField, MessageField
				>,
				ErrorEntry<
					TimePointField, MessageField, ErrorCodeField
				>
			>
	{
	private:
		CustomStringOutputLogger(StringLike auto&& loggerName,
			const Queue::Scheme queueScheme,
			const std::size_t queueCapacity);
	public:
		static std::shared_ptr<CustomStringOutputLogger> create(StringLike auto&& loggerName,
			const Queue::Scheme queueScheme = Queue::Scheme::Bound,
			const std::size_t queueCapacity = 1024);
	private:
		void createOutputs() &;
		void tune() &;
		void tuneInfoEntry() &;
		void tuneErrorEntry() &;
	private:
		std::shared_ptr<Output<>> console;
		std::shared_ptr<Output<>> file;
		std::shared_ptr<Output<>> debug;
		std::shared_ptr<Output<>> windowsEvent;
	};
}

export namespace Glas::Test
{
	CustomStringOutputLogger::CustomStringOutputLogger(StringLike auto&& loggerName,
		const Queue::Scheme queueScheme, const std::size_t queueCapacity) :

		Logger<
			InfoEntry<
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

	std::shared_ptr<CustomStringOutputLogger> CustomStringOutputLogger::create(
		StringLike auto&& loggerName, const Queue::Scheme queueScheme,
		const std::size_t queueCapacity)
	{
		auto ptr = std::unique_ptr<CustomStringOutputLogger>{
			new CustomStringOutputLogger(std::forward<decltype(loggerName)>(loggerName), queueScheme,
				queueCapacity)
		};
		return std::shared_ptr<CustomStringOutputLogger>(std::move(ptr));
	}

	void CustomStringOutputLogger::createOutputs() & {
		console = ConsoleStringOutput::create(
			ConsoleDesc{
				.consoleTitle{ L"CustomStringOutputLogger" },
				.fontFaceName{ L"Cascadia Mono" }
			}
		);

		file = FileStringOutput::create("CustomStringOutputLogger.txt",
			FileStringOutput::PathType::Relative);

		debug = DebugStringOutput::create();

		windowsEvent = Test::WindowsEventOutput::create();
	}

	void CustomStringOutputLogger::tune() & {
		tuneInfoEntry();
		tuneErrorEntry();
	}

	void CustomStringOutputLogger::tuneInfoEntry() & {
		this->InfoEntry::TimePointField::enable();
		this->InfoEntry::MessageField::enable();

		this->InfoEntry::TimePointField::padding(Padding{ .spaceAfter{ 2 } });
		this->InfoEntry::MessageField::padding(Padding{ .breakAfter{ 1 } });

		this->InfoEntry::outputs(file, console, windowsEvent);
	}

	void CustomStringOutputLogger::tuneErrorEntry() & {
		this->ErrorEntry::TimePointField::enable();
		this->ErrorEntry::MessageField::enable();
		this->ErrorEntry::ErrorCodeField::enable();

		this->ErrorEntry::TimePointField::padding(Padding{ .spaceAfter{ 2 } });
		this->ErrorEntry::MessageField::padding(Padding{ .breakAfter{ 1 } });
		this->ErrorEntry::ErrorCodeField::padding(Padding{ .breakAfter{ 1 } });

		this->ErrorEntry::outputs(debug, file, console, windowsEvent);
	}
}