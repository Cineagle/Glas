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
			Glas::Logger<
				Glas::InfoEntry<
					Glas::TimePointField, Glas::MessageField
				>,
				Glas::WarningEntry<
					Glas::TimePointField, Glas::MessageField
				>,
				Glas::ErrorEntry<
					Glas::TimePointField, Glas::MessageField, Glas::ErrorCodeField
				>,
				ValueEntry
			>
	{
	private:
		BinaryOutputLogger(Glas::StringLike auto&& loggerName,
			const Glas::Queue::Scheme queueScheme,
			const std::size_t queueCapacity);
	public:
		static std::shared_ptr<BinaryOutputLogger> create(Glas::StringLike auto&& loggerName,
			const Glas::Queue::Scheme queueScheme = Glas::Queue::Scheme::Bound,
			const std::size_t queueCapacity = 1024);
	private:
		void createOutputs() &;
		void tune() &;
		void tuneInfoEntry() &;
		void tuneWarningEntry() &;
		void tuneErrorEntry() &;
		void tuneValueEntry() &;
	private:
		std::shared_ptr<Glas::Output<>> console;
		std::shared_ptr<Glas::Output<>> file;
		std::shared_ptr<Glas::Output<>> debug;
		std::shared_ptr<Glas::Output<std::vector<std::byte>>> binaryFile;
	};
}

export namespace Glas::Test
{
	BinaryOutputLogger::BinaryOutputLogger(Glas::StringLike auto&& loggerName,
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
			>,
			ValueEntry
		>{ std::forward<decltype(loggerName)>(loggerName), queueScheme, queueCapacity }
	{
		createOutputs();
		tune();
	}

	std::shared_ptr<BinaryOutputLogger> BinaryOutputLogger::create(
		Glas::StringLike auto&& loggerName, const Glas::Queue::Scheme queueScheme,
		const std::size_t queueCapacity)
	{
		auto ptr = std::unique_ptr<BinaryOutputLogger>{
			new BinaryOutputLogger(std::forward<decltype(loggerName)>(loggerName), queueScheme,
				queueCapacity)
		};
		return std::shared_ptr<BinaryOutputLogger>(std::move(ptr));
	}

	void BinaryOutputLogger::createOutputs() & {
		console = Glas::ConsoleStringOutput::create(
			Glas::ConsoleConfig{
				.consoleTitle{ L"BinaryOutputLogger" },
				.fontFaceName{ L"Cascadia Mono" }
			}
		);

		file = Glas::FileStringOutput::create("BinaryOutputLogger 1.txt",
			Glas::FileStringOutput::PathType::Relative);

		debug = Glas::DebugStringOutput::create();

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

		this->InfoEntry::TimePointField::padding(Glas::Padding{ .spaceAfter{ 2 } });
		this->InfoEntry::MessageField::padding(Glas::Padding{ .breakAfter{ 1 } });

		this->InfoEntry::outputs(file, console);
	}

	void BinaryOutputLogger::tuneWarningEntry() & {
		this->WarningEntry::TimePointField::enable();
		this->WarningEntry::MessageField::enable();

		this->WarningEntry::TimePointField::padding(Glas::Padding{ .spaceAfter{ 2 } });
		this->WarningEntry::MessageField::padding(Glas::Padding{ .breakAfter{ 1 } });

		this->WarningEntry::outputs(file, console);
	}

	void BinaryOutputLogger::tuneErrorEntry() & {
		this->ErrorEntry::TimePointField::enable();
		this->ErrorEntry::MessageField::enable();
		this->ErrorEntry::ErrorCodeField::enable();

		this->ErrorEntry::TimePointField::padding(Glas::Padding{ .spaceAfter{ 2 } });
		this->ErrorEntry::MessageField::padding(Glas::Padding{ .breakAfter{ 1 } });
		this->ErrorEntry::ErrorCodeField::padding(Glas::Padding{ .breakAfter{ 1 } });

		this->ErrorEntry::outputs(debug, file, console);
	}

	void BinaryOutputLogger::tuneValueEntry() & {
		this->ValueEntry::outputs(binaryFile);
	}
}