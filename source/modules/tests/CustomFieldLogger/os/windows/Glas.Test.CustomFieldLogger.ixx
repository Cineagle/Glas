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
			Glas::Logger<
				Glas::InfoEntry<
					Glas::TimePointField, UserNameField, Glas::MessageField
				>,
				Glas::ErrorEntry<
					Glas::TimePointField, Glas::MessageField, Glas::ErrorCodeField
				>
			>
	{
	private:
		CustomFieldLogger(Glas::StringLike auto&& loggerName,
			const Glas::Queue::Scheme queueScheme,
			const std::size_t queueCapacity);
	public:
		static std::shared_ptr<CustomFieldLogger> create(Glas::StringLike auto&& loggerName,
			const Glas::Queue::Scheme queueScheme = Glas::Queue::Scheme::Bound,
			const std::size_t queueCapacity = 1024);
	private:
		void createOutputs() &;
		void tune() &;
		void tuneInfoEntry() &;
		void tuneErrorEntry() &;
	private:
		std::shared_ptr<Glas::Output<>> console;
		std::shared_ptr<Glas::Output<>> file;
		std::shared_ptr<Glas::Output<>> debug;
	};
}

export namespace Glas::Test
{
	CustomFieldLogger::CustomFieldLogger(Glas::StringLike auto&& loggerName,
		const Glas::Queue::Scheme queueScheme, const std::size_t queueCapacity) :

		Glas::Logger<
			Glas::InfoEntry<
				Glas::TimePointField, UserNameField, Glas::MessageField
			>,
			Glas::ErrorEntry<
				Glas::TimePointField, Glas::MessageField, Glas::ErrorCodeField
			>
		>{ std::forward<decltype(loggerName)>(loggerName), queueScheme, queueCapacity }
	{
		createOutputs();
		tune();
	}

	std::shared_ptr<CustomFieldLogger> CustomFieldLogger::create(
		Glas::StringLike auto&& loggerName, const Glas::Queue::Scheme queueScheme,
		const std::size_t queueCapacity)
	{
		auto ptr = std::unique_ptr<CustomFieldLogger>{
			new CustomFieldLogger(std::forward<decltype(loggerName)>(loggerName), queueScheme,
				queueCapacity)
		};
		return std::shared_ptr<CustomFieldLogger>(std::move(ptr));
	}

	void CustomFieldLogger::createOutputs() & {
		console = Glas::ConsoleStringOutput::create(
			Glas::ConsoleConfig{
				.consoleTitle{ L"CustomFieldLogger" },
				.fontFaceName{ L"Cascadia Mono" }
			}
		);

		file = Glas::FileStringOutput::create("CustomFieldLogger.txt",
			Glas::FileStringOutput::PathType::Relative);

		debug = Glas::DebugStringOutput::create();
	}

	void CustomFieldLogger::tune() & {
		tuneInfoEntry();
		tuneErrorEntry();
	}

	void CustomFieldLogger::tuneInfoEntry() & {
		this->InfoEntry::TimePointField::enable();
		this->InfoEntry::UserNameField::enable();
		this->InfoEntry::MessageField::enable();

		this->InfoEntry::TimePointField::padding(Glas::Padding{ .spaceAfter{ 2 } });
		this->InfoEntry::UserNameField::padding(Glas::Padding{ .spaceAfter{ 2 } });
		this->InfoEntry::MessageField::padding(Glas::Padding{ .breakAfter{ 1 } });

		this->InfoEntry::outputs(file, console);
	}

	void CustomFieldLogger::tuneErrorEntry() & {
		this->ErrorEntry::TimePointField::enable();
		this->ErrorEntry::MessageField::enable();
		this->ErrorEntry::ErrorCodeField::enable();

		this->ErrorEntry::TimePointField::padding(Glas::Padding{ .spaceAfter{ 2 } });
		this->ErrorEntry::MessageField::padding(Glas::Padding{ .breakAfter{ 1 } });
		this->ErrorEntry::ErrorCodeField::padding(Glas::Padding{ .breakAfter{ 1 } });

		this->ErrorEntry::outputs(debug, file, console);
	}
}