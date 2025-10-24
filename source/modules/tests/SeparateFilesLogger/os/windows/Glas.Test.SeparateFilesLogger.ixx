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
			Glas::Logger<
				Glas::InfoEntry<
					Glas::TimePointField, Glas::MessageField
				>,
				Glas::ErrorEntry<
					Glas::TimePointField, Glas::MessageField, Glas::ErrorCodeField
				>
			>
	{
	private:
		SeparateFilesLogger(Glas::StringLike auto&& loggerName,
			const Glas::Queue::Scheme queueScheme,
			const std::size_t queueCapacity);
	public:
		static std::shared_ptr<SeparateFilesLogger> create(Glas::StringLike auto&& loggerName,
			const Glas::Queue::Scheme queueScheme = Glas::Queue::Scheme::Bound,
			const std::size_t queueCapacity = 1024);
	private:
		void createOutputs() &;
		void tune() &;
		void tuneInfoEntry() &;
		void tuneErrorEntry() &;
	private:
		std::shared_ptr<Glas::Output<>> console;
		std::shared_ptr<Glas::Output<>> infoEntryFile;
		std::shared_ptr<Glas::Output<>> errorEntryFile;
		std::shared_ptr<Glas::Output<>> debug;
	};
}

export namespace Glas::Test
{
	SeparateFilesLogger::SeparateFilesLogger(Glas::StringLike auto&& loggerName,
		const Glas::Queue::Scheme queueScheme, const std::size_t queueCapacity) :

		Glas::Logger<
			Glas::InfoEntry<
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

	std::shared_ptr<SeparateFilesLogger> SeparateFilesLogger::create(
		Glas::StringLike auto&& loggerName, const Glas::Queue::Scheme queueScheme,
		const std::size_t queueCapacity)
	{
		auto ptr = std::unique_ptr<SeparateFilesLogger>{
			new SeparateFilesLogger(std::forward<decltype(loggerName)>(loggerName), queueScheme,
				queueCapacity)
		};
		return std::shared_ptr<SeparateFilesLogger>(std::move(ptr));
	}

	void SeparateFilesLogger::createOutputs() & {
		console = Glas::ConsoleStringOutput::create(
			Glas::ConsoleConfig{
				.consoleTitle{ L"SeparateFilesLogger" },
				.fontFaceName{ L"Cascadia Mono" }
			}
		);

		infoEntryFile = Glas::FileStringOutput::create("SeparateFilesLogger 1.txt",
			Glas::FileStringOutput::PathType::Relative);
		errorEntryFile = Glas::FileStringOutput::create("SeparateFilesLogger 2.txt",
			Glas::FileStringOutput::PathType::Relative);

		debug = Glas::DebugStringOutput::create();
	}

	void SeparateFilesLogger::tune() & {
		tuneInfoEntry();
		tuneErrorEntry();
	}

	void SeparateFilesLogger::tuneInfoEntry() & {
		this->InfoEntry::TimePointField::enable();
		this->InfoEntry::MessageField::enable();

		this->InfoEntry::TimePointField::padding(Glas::Padding{ .spaceAfter{ 2 } });
		this->InfoEntry::MessageField::padding(Glas::Padding{ .breakAfter{ 1 } });

		this->InfoEntry::outputs(infoEntryFile, console);
	}

	void SeparateFilesLogger::tuneErrorEntry() & {
		this->ErrorEntry::TimePointField::enable();
		this->ErrorEntry::MessageField::enable();
		this->ErrorEntry::ErrorCodeField::enable();

		this->ErrorEntry::TimePointField::padding(Glas::Padding{ .spaceAfter{ 2 } });
		this->ErrorEntry::MessageField::padding(Glas::Padding{ .breakAfter{ 1 } });
		this->ErrorEntry::ErrorCodeField::padding(Glas::Padding{ .breakAfter{ 1 } });

		this->ErrorEntry::outputs(debug, errorEntryFile, console);
	}
}