export module Glas.Test.ColoredLogger;
export import Glas.Logger;

export import Glas.InfoEntry;
export import Glas.WarningEntry;
export import Glas.ErrorEntry;

export import Glas.TimePointField;
export import Glas.LoggerNameField;
export import Glas.TypeField;
export import Glas.ThreadIDField;
export import Glas.MessageField;
export import Glas.ErrorCodeField;

export import Glas.FileStringOutput;
export import Glas.ConsoleStringOutput;
export import Glas.DebugStringOutput;

export import std;


export namespace Glas::Test
{
	class ColoredLogger :
		public
			Logger<
				InfoEntry<
					TimePointField, TypeField, LoggerNameField,
					ThreadIDField, MessageField
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
		ColoredLogger(StringLike auto&& loggerName,
			const Queue::Scheme queueScheme,
			const std::size_t queueCapacity);
	public:
		static std::shared_ptr<ColoredLogger> create(StringLike auto&& loggerName,
			const Queue::Scheme queueScheme = Queue::Scheme::Bound,
			const std::size_t queueCapacity = 1024);
	public:
		void changeInfoEntryStyle() &;
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
	ColoredLogger::ColoredLogger(StringLike auto&& loggerName,
		const Queue::Scheme queueScheme, const std::size_t queueCapacity) :

			Logger<
				InfoEntry<
					TimePointField, TypeField, LoggerNameField,
					ThreadIDField, MessageField
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

	std::shared_ptr<ColoredLogger> ColoredLogger::create(
		StringLike auto&& loggerName, const Queue::Scheme queueScheme,
		const std::size_t queueCapacity)
	{
		auto ptr = std::unique_ptr<ColoredLogger>{
			new ColoredLogger(std::forward<decltype(loggerName)>(loggerName), queueScheme,
				queueCapacity)
		};
		return std::shared_ptr<ColoredLogger>(std::move(ptr));
	}

	void ColoredLogger::createOutputs() & {
		console = ConsoleStringOutput::create(
			ConsoleConfig{
				.consoleTitle{ L"ColoredLogger" },
				.fontFaceName{ L"Cascadia Mono" },
				.vtEnabled{ true }
			}
		);

		file = FileStringOutput::create("ColoredLogger.txt",
			FileStringOutput::PathType::Relative);

		debug = DebugStringOutput::create();
	}

	void ColoredLogger::tune() & {
		tuneInfoEntry();
		tuneWarningEntry();
		tuneErrorEntry();
	}

	void ColoredLogger::tuneInfoEntry() & {
		this->InfoEntry::TimePointField::enable();
		this->InfoEntry::TypeField::enable();
		this->InfoEntry::LoggerNameField::enable();
		this->InfoEntry::ThreadIDField::enable();
		this->InfoEntry::MessageField::enable();

		this->InfoEntry::TimePointField::padding(Padding{ .spaceAfter{ 2 } });
		this->InfoEntry::TypeField::padding(Padding{ .spaceAfter{ 2 } });
		this->InfoEntry::LoggerNameField::padding(Padding{ .spaceAfter{ 2 } });
		this->InfoEntry::ThreadIDField::padding(Padding{ .spaceAfter{ 2 } });
		this->InfoEntry::MessageField::padding(Padding{ .breakAfter{ 1 } });

		this->InfoEntry::TimePointField::VTSequence::enable();
		this->InfoEntry::TypeField::VTSequence::enable();
		this->InfoEntry::LoggerNameField::VTSequence::enable();
		this->InfoEntry::ThreadIDField::VTSequence::enable();
		this->InfoEntry::MessageField::VTSequence::enable();

		this->InfoEntry::outputs(file, console);
	}

	void ColoredLogger::tuneWarningEntry() & {
		this->WarningEntry::TimePointField::enable();
		this->WarningEntry::MessageField::enable();

		this->WarningEntry::TimePointField::padding(Padding{ .spaceAfter{ 2 } });
		this->WarningEntry::MessageField::padding(Padding{ .breakAfter{ 1 } });

		this->WarningEntry::TimePointField::VTSequence::enable();
		this->WarningEntry::MessageField::VTSequence::enable();

		this->WarningEntry::outputs(file, console);
	}

	void ColoredLogger::tuneErrorEntry() & {
		this->ErrorEntry::TimePointField::enable();
		this->ErrorEntry::MessageField::enable();
		this->ErrorEntry::ErrorCodeField::enable();

		this->ErrorEntry::TimePointField::padding(Padding{ .spaceAfter{ 2 } });
		this->ErrorEntry::MessageField::padding(Padding{ .breakAfter{ 1 } });
		this->ErrorEntry::ErrorCodeField::padding(Padding{ .breakAfter{ 1 } });

		this->ErrorEntry::TimePointField::VTSequence::enable();
		this->ErrorEntry::MessageField::VTSequence::enable();
		this->ErrorEntry::ErrorCodeField::VTSequence::enable();

		this->ErrorEntry::outputs(debug, file, console);
	}

	void ColoredLogger::changeInfoEntryStyle() & {
		this->InfoEntry::TimePointField::style(
			VTStyle{
				.fgColor{ .red{ 144 }, .green{ 144 }, .blue{ 200 } }
			}
		);

		this->InfoEntry::TypeField::style(
			VTStyle{
				.fgColor{.red{ 200 }, .green{ 77 }, .blue{ 200 } },
				.bgColor{.red{ 20 }, .green{ 20 }, .blue{ 20 } },
			}
		);

		this->InfoEntry::LoggerNameField::style(
			VTStyle{
				.fgColor{.red{ 144 }, .green{ 225 }, .blue{ 145 } },
				.bgColor{.red{ 50 }, .green{ 20 }, .blue{ 20 } },
				.effect{ VTStyle::Effect::Italic }
			}
		);
	}
}