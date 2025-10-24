export module Glas.Test.MaxiLogger;
export import Glas.Logger;

export import Glas.InfoEntry;
export import Glas.SuccessEntry;
export import Glas.WarningEntry;
export import Glas.ErrorEntry;
export import Glas.LinesEntry;
export import Glas.SequenceEntry;
export import Glas.TraceEntry;
export import Glas.BytesEntry;

export import Glas.TimePointField;
export import Glas.TypeField;
export import Glas.LoggerNameField;
export import Glas.ThreadIDField;
export import Glas.MessageField;
export import Glas.ErrorCodeField;
export import Glas.LocationField;

export import Glas.FileStringOutput;
export import Glas.ConsoleStringOutput;
export import Glas.DebugStringOutput;

export import std;


export namespace Glas::Test
{
	class MaxiLogger :
		public
			Glas::Logger<
				Glas::InfoEntry<
					Glas::TimePointField, Glas::TypeField, Glas::LoggerNameField,
					Glas::ThreadIDField, Glas::MessageField, Glas::ErrorCodeField,
					Glas::LocationField
				>,
				Glas::SuccessEntry<
					Glas::TimePointField, Glas::TypeField, Glas::LoggerNameField,
					Glas::ThreadIDField, Glas::MessageField, Glas::ErrorCodeField,
					Glas::LocationField
				>,
				Glas::WarningEntry<
					Glas::TimePointField, Glas::TypeField, Glas::LoggerNameField,
					Glas::ThreadIDField, Glas::MessageField, Glas::ErrorCodeField,
					Glas::LocationField
				>,
				Glas::ErrorEntry<
					Glas::TimePointField, Glas::TypeField, Glas::LoggerNameField,
					Glas::ThreadIDField, Glas::MessageField, Glas::ErrorCodeField,
					Glas::LocationField
				>,
				Glas::LinesEntry<
					Glas::MessageField 
					/* 
						We can also add all other fields, but this is not nessasary.
						for the LinesEntry.
					*/
				>,
				Glas::SequenceEntry<
					Glas::MessageField 
					/* 
						We can also add all other fields, but this is not nessasary.
						for the SequenceEntry.
					*/
				>,
				Glas::TraceEntry<
					Glas::MessageField 
					/* 
						We can also add all other fields, but this is not nessasary.
						for the TraceEntry.
					*/
				>,
				Glas::BytesEntry<
					Glas::MessageField 
					/* 
						We can also add all other fields, but this is not nessasary.
						for the BytesEntry.
					*/
				>
			>
	{
	private:
		MaxiLogger(Glas::StringLike auto&& loggerName,
			const Glas::Queue::Scheme queueScheme,
			const std::size_t queueCapacity);
	public:
		static std::shared_ptr<MaxiLogger> create(Glas::StringLike auto&& loggerName,
			const Glas::Queue::Scheme queueScheme = Glas::Queue::Scheme::Bound,
			const std::size_t queueCapacity = 1024);
	private:
		void createOutputs() &;
		void tune() &;
		void tuneInfoEntry() &;
		void tuneSuccessEntry() &;
		void tuneWarningEntry() &;
		void tuneErrorEntry() &;
		void tuneLinesEntry() &;
		void tuneSequenceEntry() &;
		void tuneTraceEntry() &;
		void tuneBytesEntry() &;
	private:
		std::shared_ptr<Glas::Output<>> console;
		std::shared_ptr<Glas::Output<>> file;
		std::shared_ptr<Glas::Output<>> debug;
	};
}

export namespace Glas::Test
{
	MaxiLogger::MaxiLogger(Glas::StringLike auto&& loggerName,
		const Glas::Queue::Scheme queueScheme, const std::size_t queueCapacity) :

			Glas::Logger<
				Glas::InfoEntry<
					Glas::TimePointField, Glas::TypeField, Glas::LoggerNameField,
					Glas::ThreadIDField, Glas::MessageField, Glas::ErrorCodeField,
					Glas::LocationField
				>,
				Glas::SuccessEntry<
					Glas::TimePointField, Glas::TypeField, Glas::LoggerNameField,
					Glas::ThreadIDField, Glas::MessageField, Glas::ErrorCodeField,
					Glas::LocationField
				>,
				Glas::WarningEntry<
					Glas::TimePointField, Glas::TypeField, Glas::LoggerNameField,
					Glas::ThreadIDField, Glas::MessageField, Glas::ErrorCodeField,
					Glas::LocationField
				>,
				Glas::ErrorEntry<
					Glas::TimePointField, Glas::TypeField, Glas::LoggerNameField,
					Glas::ThreadIDField, Glas::MessageField, Glas::ErrorCodeField,
					Glas::LocationField
				>,
				Glas::LinesEntry<
					Glas::MessageField
				>,
				Glas::SequenceEntry<
					Glas::MessageField
				>,
				Glas::TraceEntry<
					Glas::MessageField
				>,
				Glas::BytesEntry<
					Glas::MessageField
				>
		>{ std::forward<decltype(loggerName)>(loggerName), queueScheme, queueCapacity }
	{
		createOutputs();
		tune();
	}

	std::shared_ptr<MaxiLogger> MaxiLogger::create(
		Glas::StringLike auto&& loggerName, const Glas::Queue::Scheme queueScheme,
		const std::size_t queueCapacity)
	{
		auto ptr = std::unique_ptr<MaxiLogger>{
			new MaxiLogger(std::forward<decltype(loggerName)>(loggerName), queueScheme,
				queueCapacity)
		};
		return std::shared_ptr<MaxiLogger>(std::move(ptr));
	}

	void MaxiLogger::createOutputs() & {
		console = Glas::ConsoleStringOutput::create(
			Glas::ConsoleConfig{
				.consoleTitle{ L"ColoredLogger" },
				.fontFaceName{ L"Cascadia Mono" },
				.vtEnabled{ true }
			}
		);

		file = Glas::FileStringOutput::create("MaxiLogger.txt",
			Glas::FileStringOutput::PathType::Relative);

		debug = Glas::DebugStringOutput::create();
	}

	void MaxiLogger::tune() & {
		tuneInfoEntry();
		tuneSuccessEntry();
		tuneWarningEntry();
		tuneErrorEntry();
		tuneLinesEntry();
		tuneSequenceEntry();
		tuneTraceEntry();
		tuneBytesEntry();
	}

	void MaxiLogger::tuneInfoEntry() & {
		this->InfoEntry::TimePointField::enable();
		this->InfoEntry::TypeField::enable();
		this->InfoEntry::LoggerNameField::enable();
		this->InfoEntry::ThreadIDField::enable();
		this->InfoEntry::MessageField::enable();
		this->InfoEntry::ErrorCodeField::enable();
		this->InfoEntry::LocationField::enable();

		this->InfoEntry::TimePointField::padding(Glas::Padding{ .spaceAfter{ 2 } });
		this->InfoEntry::TypeField::padding(Glas::Padding{ .spaceAfter{ 2 } });
		this->InfoEntry::LoggerNameField::padding(Glas::Padding{ .spaceAfter{ 2 } });
		this->InfoEntry::ThreadIDField::padding(Glas::Padding{ .spaceAfter{ 2 } });
		this->InfoEntry::MessageField::padding(Glas::Padding{ .breakAfter{ 1 } });
		this->InfoEntry::ErrorCodeField::padding(Glas::Padding{ .breakAfter{ 1 } });
		this->InfoEntry::LocationField::padding(Glas::Padding{ .breakAfter{ 1 } });

		this->InfoEntry::TimePointField::VTSequence::enable();
		this->InfoEntry::TypeField::VTSequence::enable();
		this->InfoEntry::LoggerNameField::VTSequence::enable();
		this->InfoEntry::ThreadIDField::VTSequence::enable();
		this->InfoEntry::MessageField::VTSequence::enable();
		this->InfoEntry::ErrorCodeField::VTSequence::enable();
		this->InfoEntry::LocationField::VTSequence::enable();

		this->InfoEntry::outputs(console, file, debug);
	}

	void MaxiLogger::tuneSuccessEntry() & {
		this->SuccessEntry::TimePointField::enable();
		this->SuccessEntry::TypeField::enable();
		this->SuccessEntry::LoggerNameField::enable();
		this->SuccessEntry::ThreadIDField::enable();
		this->SuccessEntry::MessageField::enable();
		this->SuccessEntry::ErrorCodeField::enable();
		this->SuccessEntry::LocationField::enable();

		this->SuccessEntry::TimePointField::padding(Glas::Padding{ .spaceAfter{ 2 } });
		this->SuccessEntry::TypeField::padding(Glas::Padding{ .spaceAfter{ 2 } });
		this->SuccessEntry::LoggerNameField::padding(Glas::Padding{ .spaceAfter{ 2 } });
		this->SuccessEntry::ThreadIDField::padding(Glas::Padding{ .spaceAfter{ 2 } });
		this->SuccessEntry::MessageField::padding(Glas::Padding{ .breakAfter{ 1 } });
		this->SuccessEntry::ErrorCodeField::padding(Glas::Padding{ .breakAfter{ 1 } });
		this->SuccessEntry::LocationField::padding(Glas::Padding{ .breakAfter{ 1 } });

		this->SuccessEntry::TimePointField::VTSequence::enable();
		this->SuccessEntry::TypeField::VTSequence::enable();
		this->SuccessEntry::LoggerNameField::VTSequence::enable();
		this->SuccessEntry::ThreadIDField::VTSequence::enable();
		this->SuccessEntry::MessageField::VTSequence::enable();
		this->SuccessEntry::ErrorCodeField::VTSequence::enable();
		this->SuccessEntry::LocationField::VTSequence::enable();

		this->SuccessEntry::outputs(console, file, debug);
	}

	void MaxiLogger::tuneWarningEntry() & {
		this->WarningEntry::TimePointField::enable();
		this->WarningEntry::TypeField::enable();
		this->WarningEntry::LoggerNameField::enable();
		this->WarningEntry::ThreadIDField::enable();
		this->WarningEntry::MessageField::enable();
		this->WarningEntry::ErrorCodeField::enable();
		this->WarningEntry::LocationField::enable();

		this->WarningEntry::TimePointField::padding(Glas::Padding{ .spaceAfter{ 2 } });
		this->WarningEntry::TypeField::padding(Glas::Padding{ .spaceAfter{ 2 } });
		this->WarningEntry::LoggerNameField::padding(Glas::Padding{ .spaceAfter{ 2 } });
		this->WarningEntry::ThreadIDField::padding(Glas::Padding{ .spaceAfter{ 2 } });
		this->WarningEntry::MessageField::padding(Glas::Padding{ .breakAfter{ 1 } });
		this->WarningEntry::ErrorCodeField::padding(Glas::Padding{ .breakAfter{ 1 } });
		this->WarningEntry::LocationField::padding(Glas::Padding{ .breakAfter{ 1 } });

		this->WarningEntry::TimePointField::VTSequence::enable();
		this->WarningEntry::TypeField::VTSequence::enable();
		this->WarningEntry::LoggerNameField::VTSequence::enable();
		this->WarningEntry::ThreadIDField::VTSequence::enable();
		this->WarningEntry::MessageField::VTSequence::enable();
		this->WarningEntry::ErrorCodeField::VTSequence::enable();
		this->WarningEntry::LocationField::VTSequence::enable();

		this->WarningEntry::outputs(console, file, debug);
	}

	void MaxiLogger::tuneErrorEntry() & {
		this->ErrorEntry::TimePointField::enable();
		this->ErrorEntry::TypeField::enable();
		this->ErrorEntry::LoggerNameField::enable();
		this->ErrorEntry::ThreadIDField::enable();
		this->ErrorEntry::MessageField::enable();
		this->ErrorEntry::ErrorCodeField::enable();
		this->ErrorEntry::LocationField::enable();

		this->ErrorEntry::TimePointField::padding(Glas::Padding{ .spaceAfter{ 2 } });
		this->ErrorEntry::TypeField::padding(Glas::Padding{ .spaceAfter{ 2 } });
		this->ErrorEntry::LoggerNameField::padding(Glas::Padding{ .spaceAfter{ 2 } });
		this->ErrorEntry::ThreadIDField::padding(Glas::Padding{ .spaceAfter{ 2 } });
		this->ErrorEntry::MessageField::padding(Glas::Padding{ .breakAfter{ 1 } });
		this->ErrorEntry::ErrorCodeField::padding(Glas::Padding{ .breakAfter{ 1 } });
		this->ErrorEntry::LocationField::padding(Glas::Padding{ .breakAfter{ 1 } });

		this->ErrorEntry::TimePointField::VTSequence::enable();
		this->ErrorEntry::TypeField::VTSequence::enable();
		this->ErrorEntry::LoggerNameField::VTSequence::enable();
		this->ErrorEntry::ThreadIDField::VTSequence::enable();
		this->ErrorEntry::MessageField::VTSequence::enable();
		this->ErrorEntry::ErrorCodeField::VTSequence::enable();
		this->ErrorEntry::LocationField::VTSequence::enable();

		this->ErrorEntry::outputs(console, file, debug);
	}

	void MaxiLogger::tuneLinesEntry() & {
		this->LinesEntry::MessageField::enable();
		this->LinesEntry::outputs(console, file, debug);
	}

	void MaxiLogger::tuneSequenceEntry() & {
		this->SequenceEntry::MessageField::enable();
		this->SequenceEntry::MessageField::padding(Glas::Padding{ .breakAfter{ 1 } });
		this->SequenceEntry::outputs(console, file, debug);
	}

	void MaxiLogger::tuneTraceEntry() & {
		this->TraceEntry::MessageField::enable();
		this->TraceEntry::MessageField::padding(Glas::Padding{ .breakAfter{ 1 } });
		this->TraceEntry::outputs(console, file, debug);
	}

	void MaxiLogger::tuneBytesEntry() & {
		this->BytesEntry::MessageField::enable();
		this->BytesEntry::MessageField::padding(Glas::Padding{ .breakAfter{ 1 } });

		//this->BytesEntry::MessageField::VTSequence::enable();
		//this->BytesEntry::MessageField::style(
		//	Glas::VTStyle{
		//		.fgColor{.red{ 50 }, .green{ 50 }, .blue{ 200 } }
		//	}
		//);

		this->BytesEntry::outputs(console, file, debug);
	}
}