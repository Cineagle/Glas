export module Glas.Test.CustomFieldLogger;
export import Glas.Logger;

export import Glas.InfoEntry;
export import Glas.ErrorEntry;

export import Glas.TimePointField;
export import Glas.MessageField;
export import Glas.ErrorCodeField;
export import Glas.Test.UserNameField;

export import Glas.FileStringOutput;
export import Glas.ConsoleStringOutput;
export import Glas.DebugStringOutput;

export import std;


export namespace Glas::Test
{
	class CustomFieldLogger :
		public
			Logger<
				InfoEntry<
					TimePointField, UserNameField, MessageField
				>,
				ErrorEntry<
					TimePointField, MessageField, ErrorCodeField
				>
			>
	{
	private:
		CustomFieldLogger(StringLike auto&& loggerName,
			const Queue::Scheme queueScheme,
			const std::size_t queueCapacity);
	public:
		static std::shared_ptr<CustomFieldLogger> create(StringLike auto&& loggerName,
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
	};
}

export namespace Glas::Test
{
	CustomFieldLogger::CustomFieldLogger(StringLike auto&& loggerName,
		const Queue::Scheme queueScheme, const std::size_t queueCapacity) :

		Logger<
			InfoEntry<
				TimePointField, UserNameField, MessageField
			>,
			ErrorEntry<
				TimePointField, MessageField, ErrorCodeField
			>
		>{ std::forward<decltype(loggerName)>(loggerName), queueScheme, queueCapacity }
	{
		createOutputs();
		tune();
	}

	std::shared_ptr<CustomFieldLogger> CustomFieldLogger::create(
		StringLike auto&& loggerName, const Queue::Scheme queueScheme,
		const std::size_t queueCapacity)
	{
		auto ptr = std::unique_ptr<CustomFieldLogger>{
			new CustomFieldLogger(std::forward<decltype(loggerName)>(loggerName), queueScheme,
				queueCapacity)
		};
		return std::shared_ptr<CustomFieldLogger>(std::move(ptr));
	}

	void CustomFieldLogger::createOutputs() & {
		console = ConsoleStringOutput::create(
			ConsoleConfig{
				.consoleTitle{ L"CustomFieldLogger" },
				.fontFaceName{ L"Cascadia Mono" }
			}
		);

		file = FileStringOutput::create("CustomFieldLogger.txt",
			FileStringOutput::PathType::Relative);

		debug = DebugStringOutput::create();
	}

	void CustomFieldLogger::tune() & {
		tuneInfoEntry();
		tuneErrorEntry();
	}

	void CustomFieldLogger::tuneInfoEntry() & {
		this->InfoEntry::TimePointField::enable();
		this->InfoEntry::UserNameField::enable();
		this->InfoEntry::MessageField::enable();

		this->InfoEntry::TimePointField::padding(Padding{ .spaceAfter{ 2 } });
		this->InfoEntry::UserNameField::padding(Padding{ .spaceAfter{ 2 } });
		this->InfoEntry::MessageField::padding(Padding{ .breakAfter{ 1 } });

		this->InfoEntry::outputs(file, console);
	}

	void CustomFieldLogger::tuneErrorEntry() & {
		this->ErrorEntry::TimePointField::enable();
		this->ErrorEntry::MessageField::enable();
		this->ErrorEntry::ErrorCodeField::enable();

		this->ErrorEntry::TimePointField::padding(Padding{ .spaceAfter{ 2 } });
		this->ErrorEntry::MessageField::padding(Padding{ .breakAfter{ 1 } });
		this->ErrorEntry::ErrorCodeField::padding(Padding{ .breakAfter{ 1 } });

		this->ErrorEntry::outputs(debug, file, console);
	}
}