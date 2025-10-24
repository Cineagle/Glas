export module Glas.Test.BinaryOutputLogger;
export import Glas.Logger;

export import Glas.InfoEntry;
export import Glas.WarningEntry;
export import Glas.ErrorEntry;
export import Glas.Test.ValueEntry;

export import Glas.TimePointField;
export import Glas.MessageField;
export import Glas.ErrorCodeField;

export import Glas.FileStringOutput;
export import Glas.ConsoleStringOutput;
export import Glas.DebugStringOutput;
export import Glas.Test.BinaryOutputFile;

export import std;


export namespace Glas::Test
{
	class BinaryOutputLogger :
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
				>,
				ValueEntry
			>
	{
	private:
		BinaryOutputLogger(StringLike auto&& loggerName,
			const Queue::Scheme queueScheme,
			const std::size_t queueCapacity);
	public:
		static std::shared_ptr<BinaryOutputLogger> create(StringLike auto&& loggerName,
			const Queue::Scheme queueScheme = Queue::Scheme::Bound,
			const std::size_t queueCapacity = 1024);
	private:
		void createOutputs() &;
		void tune() &;
		void tuneInfoEntry() &;
		void tuneWarningEntry() &;
		void tuneErrorEntry() &;
		void tuneValueEntry() &;
	private:
		std::shared_ptr<Output<>> console;
		std::shared_ptr<Output<>> file;
		std::shared_ptr<Output<>> debug;
		std::shared_ptr<Output<std::vector<std::byte>>> binaryFile;
	};
}

export namespace Glas::Test
{
	BinaryOutputLogger::BinaryOutputLogger(StringLike auto&& loggerName,
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
			>,
			ValueEntry
		>{ std::forward<decltype(loggerName)>(loggerName), queueScheme, queueCapacity }
	{
		createOutputs();
		tune();
	}

	std::shared_ptr<BinaryOutputLogger> BinaryOutputLogger::create(
		StringLike auto&& loggerName, const Queue::Scheme queueScheme,
		const std::size_t queueCapacity)
	{
		auto ptr = std::unique_ptr<BinaryOutputLogger>{
			new BinaryOutputLogger(std::forward<decltype(loggerName)>(loggerName), queueScheme,
				queueCapacity)
		};
		return std::shared_ptr<BinaryOutputLogger>(std::move(ptr));
	}

	void BinaryOutputLogger::createOutputs() & {
		console = ConsoleStringOutput::create(
			ConsoleConfig{
				.consoleTitle{ L"BinaryOutputLogger" },
				.fontFaceName{ L"Cascadia Mono" }
			}
		);

		file = FileStringOutput::create("BinaryOutputLogger 1.txt",
			FileStringOutput::PathType::Relative);

		debug = DebugStringOutput::create();

		binaryFile = BinaryOutputFile::create("BinaryOutputLogger 2.txt", 
			BinaryOutputFile::PathType::Relative);
	}

	void BinaryOutputLogger::tune() & {
		tuneInfoEntry();
		tuneWarningEntry();
		tuneErrorEntry();
		tuneValueEntry();
	}

	void BinaryOutputLogger::tuneInfoEntry() & {
		this->InfoEntry::TimePointField::enable();
		this->InfoEntry::MessageField::enable();

		this->InfoEntry::TimePointField::padding(Padding{ .spaceAfter{ 2 } });
		this->InfoEntry::MessageField::padding(Padding{ .breakAfter{ 1 } });

		this->InfoEntry::outputs(file, console);
	}

	void BinaryOutputLogger::tuneWarningEntry() & {
		this->WarningEntry::TimePointField::enable();
		this->WarningEntry::MessageField::enable();

		this->WarningEntry::TimePointField::padding(Padding{ .spaceAfter{ 2 } });
		this->WarningEntry::MessageField::padding(Padding{ .breakAfter{ 1 } });

		this->WarningEntry::outputs(file, console);
	}

	void BinaryOutputLogger::tuneErrorEntry() & {
		this->ErrorEntry::TimePointField::enable();
		this->ErrorEntry::MessageField::enable();
		this->ErrorEntry::ErrorCodeField::enable();

		this->ErrorEntry::TimePointField::padding(Padding{ .spaceAfter{ 2 } });
		this->ErrorEntry::MessageField::padding(Padding{ .breakAfter{ 1 } });
		this->ErrorEntry::ErrorCodeField::padding(Padding{ .breakAfter{ 1 } });

		this->ErrorEntry::outputs(debug, file, console);
	}

	void BinaryOutputLogger::tuneValueEntry() & {
		this->ValueEntry::outputs(binaryFile);
	}
}