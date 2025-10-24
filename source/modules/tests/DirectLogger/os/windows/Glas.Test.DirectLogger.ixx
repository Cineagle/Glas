export module Glas.Test.DirectLogger;
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
	class DirectLogger :
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
	private:
		DirectLogger(StringLike auto&& loggerName,
			const Glas::Queue::Scheme queueScheme,
			const std::size_t queueCapacity);
	public:
		static std::shared_ptr<DirectLogger> create(StringLike auto&& loggerName,
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
	DirectLogger::DirectLogger(StringLike auto&& loggerName,
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

	std::shared_ptr<DirectLogger> DirectLogger::create(
		StringLike auto&& loggerName, const Glas::Queue::Scheme queueScheme,
		const std::size_t queueCapacity)
	{
		auto ptr = std::unique_ptr<DirectLogger>{
			new DirectLogger(std::forward<decltype(loggerName)>(loggerName), queueScheme,
				queueCapacity)
		};
		return std::shared_ptr<DirectLogger>(std::move(ptr));
	}

	void DirectLogger::createOutputs() & {
		console = Glas::ConsoleStringOutput::create(
			Glas::ConsoleConfig{
				.consoleTitle{ L"DirectLogger" },
				.fontFaceName{ L"Cascadia Mono" }
			}
		);

		file = Glas::FileStringOutput::create("DirectLogger.txt",
			Glas::FileStringOutput::PathType::Relative);

		debug = Glas::DebugStringOutput::create();
	}

	void DirectLogger::tune() & {
		tuneInfoEntry();
		tuneWarningEntry();
		tuneErrorEntry();
	}

	void DirectLogger::tuneInfoEntry() & {
		this->InfoEntry::TimePointField::enable();
		this->InfoEntry::MessageField::enable();

		this->InfoEntry::TimePointField::padding(Glas::Padding{ .spaceAfter{ 2 } });
		this->InfoEntry::MessageField::padding(Glas::Padding{ .breakAfter{ 1 } });

		this->InfoEntry::outputs(file, console);
	}

	void DirectLogger::tuneWarningEntry() & {
		this->WarningEntry::TimePointField::enable();
		this->WarningEntry::MessageField::enable();

		this->WarningEntry::TimePointField::padding(Glas::Padding{ .spaceAfter{ 2 } });
		this->WarningEntry::MessageField::padding(Glas::Padding{ .breakAfter{ 1 } });

		this->WarningEntry::outputs(file, console);

		this->WarningEntry::scheme(Glas::Entry::Scheme::Direct);
	}

	void DirectLogger::tuneErrorEntry() & {
		this->ErrorEntry::TimePointField::enable();
		this->ErrorEntry::MessageField::enable();
		this->ErrorEntry::ErrorCodeField::enable();

		this->ErrorEntry::TimePointField::padding(Glas::Padding{ .spaceAfter{ 2 } });
		this->ErrorEntry::MessageField::padding(Glas::Padding{ .breakAfter{ 1 } });
		this->ErrorEntry::ErrorCodeField::padding(Glas::Padding{ .breakAfter{ 1 } });

		this->ErrorEntry::outputs(debug, file, console);

		this->ErrorEntry::scheme(Glas::Entry::Scheme::Direct);
	}
}