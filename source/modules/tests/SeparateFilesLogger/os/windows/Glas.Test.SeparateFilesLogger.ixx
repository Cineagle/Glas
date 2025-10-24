export module Glas.Test.SeparateFilesLogger;
export import Glas.Logger;

export import Glas.InfoEntry;
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
	class SeparateFilesLogger :
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
		SeparateFilesLogger(StringLike auto&& loggerName,
			const Queue::Scheme queueScheme,
			const std::size_t queueCapacity);
	public:
		static std::shared_ptr<SeparateFilesLogger> create(StringLike auto&& loggerName,
			const Queue::Scheme queueScheme = Queue::Scheme::Bound,
			const std::size_t queueCapacity = 1024);
	private:
		void createOutputs() &;
		void tune() &;
		void tuneInfoEntry() &;
		void tuneErrorEntry() &;
	private:
		std::shared_ptr<Output<>> console;
		std::shared_ptr<Output<>> infoEntryFile;
		std::shared_ptr<Output<>> errorEntryFile;
		std::shared_ptr<Output<>> debug;
	};
}

export namespace Glas::Test
{
	SeparateFilesLogger::SeparateFilesLogger(StringLike auto&& loggerName,
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

	std::shared_ptr<SeparateFilesLogger> SeparateFilesLogger::create(
		StringLike auto&& loggerName, const Queue::Scheme queueScheme,
		const std::size_t queueCapacity)
	{
		auto ptr = std::unique_ptr<SeparateFilesLogger>{
			new SeparateFilesLogger(std::forward<decltype(loggerName)>(loggerName), queueScheme,
				queueCapacity)
		};
		return std::shared_ptr<SeparateFilesLogger>(std::move(ptr));
	}

	void SeparateFilesLogger::createOutputs() & {
		console = ConsoleStringOutput::create(
			ConsoleConfig{
				.consoleTitle{ L"SeparateFilesLogger" },
				.fontFaceName{ L"Cascadia Mono" }
			}
		);

		infoEntryFile = FileStringOutput::create("SeparateFilesLogger 1.txt",
			FileStringOutput::PathType::Relative);
		errorEntryFile = FileStringOutput::create("SeparateFilesLogger 2.txt",
			FileStringOutput::PathType::Relative);

		debug = DebugStringOutput::create();
	}

	void SeparateFilesLogger::tune() & {
		tuneInfoEntry();
		tuneErrorEntry();
	}

	void SeparateFilesLogger::tuneInfoEntry() & {
		this->InfoEntry::TimePointField::enable();
		this->InfoEntry::MessageField::enable();

		this->InfoEntry::TimePointField::padding(Padding{ .spaceAfter{ 2 } });
		this->InfoEntry::MessageField::padding(Padding{ .breakAfter{ 1 } });

		this->InfoEntry::outputs(infoEntryFile, console);
	}

	void SeparateFilesLogger::tuneErrorEntry() & {
		this->ErrorEntry::TimePointField::enable();
		this->ErrorEntry::MessageField::enable();
		this->ErrorEntry::ErrorCodeField::enable();

		this->ErrorEntry::TimePointField::padding(Padding{ .spaceAfter{ 2 } });
		this->ErrorEntry::MessageField::padding(Padding{ .breakAfter{ 1 } });
		this->ErrorEntry::ErrorCodeField::padding(Padding{ .breakAfter{ 1 } });

		this->ErrorEntry::outputs(debug, errorEntryFile, console);
	}
}