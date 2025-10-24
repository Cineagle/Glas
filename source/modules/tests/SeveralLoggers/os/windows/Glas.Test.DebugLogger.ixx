export module Glas.Test.DebugLogger;
export import Glas.Logger;

export import Glas.SuccessEntry;
export import Glas.WarningEntry;
export import Glas.ErrorEntry;

export import Glas.TimePointField;
export import Glas.TypeField;
export import Glas.LoggerNameField;
export import Glas.MessageField;
export import Glas.ErrorCodeField;
export import Glas.LocationField;

export import std;


export namespace Glas::Test
{
	class DebugLogger :
		public
			Glas::Logger<
				Glas::SuccessEntry<
					Glas::TimePointField, Glas::TypeField, Glas::LoggerNameField, 
					Glas::MessageField
				>,
				Glas::WarningEntry<
					Glas::TimePointField, Glas::TypeField, Glas::LoggerNameField,
					Glas::MessageField
				>,
				Glas::ErrorEntry<
					Glas::TimePointField, Glas::TypeField, Glas::LoggerNameField,
					Glas::MessageField, Glas::ErrorCodeField,	Glas::LocationField
				>
			>
	{
	public:
		using Success = Glas::SuccessEntry<
			Glas::TimePointField, Glas::TypeField, Glas::LoggerNameField,
			Glas::MessageField
		>;
		using Warning = Glas::WarningEntry<
			Glas::TimePointField, Glas::TypeField, Glas::LoggerNameField,
			Glas::MessageField
		>;
		using Error = Glas::ErrorEntry<
			Glas::TimePointField, Glas::TypeField, Glas::LoggerNameField,
			Glas::MessageField, Glas::ErrorCodeField, Glas::LocationField
		>;
	private:
		DebugLogger(StringLike auto&& loggerName,
			const Glas::Queue::Scheme queueScheme,
			const std::size_t queueCapacity);
	public:
		static std::shared_ptr<DebugLogger> create(StringLike auto&& loggerName,
			const Glas::Queue::Scheme queueScheme = Glas::Queue::Scheme::Bound,
			const std::size_t queueCapacity = 1024);
	private:
		void tune() &;
		void tuneSuccessEntry() &;
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
	DebugLogger::DebugLogger(StringLike auto&& loggerName,
		const Glas::Queue::Scheme queueScheme, const std::size_t queueCapacity) :

			Glas::Logger<
				Glas::SuccessEntry<
					Glas::TimePointField, Glas::TypeField, Glas::LoggerNameField, 
					Glas::MessageField
				>,
				Glas::WarningEntry<
					Glas::TimePointField, Glas::TypeField, Glas::LoggerNameField,
					Glas::MessageField
				>,
				Glas::ErrorEntry<
					Glas::TimePointField, Glas::TypeField, Glas::LoggerNameField,
					Glas::MessageField, Glas::ErrorCodeField,	Glas::LocationField
				>
		>{ std::forward<decltype(loggerName)>(loggerName), queueScheme, queueCapacity }
	{
		tune();
	}

	std::shared_ptr<DebugLogger> DebugLogger::create(
		StringLike auto&& loggerName, const Glas::Queue::Scheme queueScheme,
		const std::size_t queueCapacity)
	{
		auto ptr = std::unique_ptr<DebugLogger>{
			new DebugLogger(std::forward<decltype(loggerName)>(loggerName), queueScheme,
				queueCapacity)
		};
		return std::shared_ptr<DebugLogger>(std::move(ptr));
	}

	void DebugLogger::tune() & {
		tuneSuccessEntry();
		tuneWarningEntry();
		tuneErrorEntry();
	}

	void DebugLogger::tuneSuccessEntry() & {
		this->SuccessEntry::TimePointField::enable();
		this->SuccessEntry::TypeField::enable();
		this->SuccessEntry::LoggerNameField::enable();
		this->SuccessEntry::MessageField::enable();

		this->SuccessEntry::TimePointField::padding(Glas::Padding{ .spaceAfter{ 2 } });
		this->SuccessEntry::TypeField::padding(Glas::Padding{ .spaceAfter{ 2 } });
		this->SuccessEntry::LoggerNameField::padding(Glas::Padding{ .spaceAfter{ 2 } });
		this->SuccessEntry::MessageField::padding(Glas::Padding{ .breakAfter{ 1 } });

		this->SuccessEntry::TimePointField::VTSequence::enable();
		this->SuccessEntry::TypeField::VTSequence::enable();
		this->SuccessEntry::LoggerNameField::VTSequence::enable();
		this->SuccessEntry::MessageField::VTSequence::enable();

		this->SuccessEntry::scheme(Glas::Entry::Scheme::Direct);
	}

	void DebugLogger::tuneWarningEntry() & {
		this->WarningEntry::TimePointField::enable();
		this->WarningEntry::TypeField::enable();
		this->WarningEntry::LoggerNameField::enable();
		this->WarningEntry::MessageField::enable();

		this->WarningEntry::TimePointField::padding(Glas::Padding{ .spaceAfter{ 2 } });
		this->WarningEntry::TypeField::padding(Glas::Padding{ .spaceAfter{ 2 } });
		this->WarningEntry::LoggerNameField::padding(Glas::Padding{ .spaceAfter{ 2 } });
		this->WarningEntry::MessageField::padding(Glas::Padding{ .breakAfter{ 1 } });

		this->WarningEntry::TimePointField::VTSequence::enable();
		this->WarningEntry::TypeField::VTSequence::enable();
		this->WarningEntry::LoggerNameField::VTSequence::enable();
		this->WarningEntry::MessageField::VTSequence::enable();
	}

	void DebugLogger::tuneErrorEntry() & {
		this->ErrorEntry::TimePointField::enable();
		this->ErrorEntry::TypeField::enable();
		this->ErrorEntry::LoggerNameField::enable();
		this->ErrorEntry::MessageField::enable();
		this->ErrorEntry::ErrorCodeField::enable();
		this->ErrorEntry::LocationField::enable();

		this->ErrorEntry::TimePointField::padding(Glas::Padding{ .spaceAfter{ 2 } });
		this->ErrorEntry::TypeField::padding(Glas::Padding{ .spaceAfter{ 2 } });
		this->ErrorEntry::LoggerNameField::padding(Glas::Padding{ .spaceAfter{ 2 } });
		this->ErrorEntry::MessageField::padding(Glas::Padding{ .breakAfter{ 1 } });
		this->ErrorEntry::ErrorCodeField::padding(Glas::Padding{ .breakAfter{ 1 } });
		this->ErrorEntry::LocationField::padding(Glas::Padding{ .breakAfter{ 1 } });

		this->ErrorEntry::TimePointField::VTSequence::enable();
		this->ErrorEntry::TypeField::VTSequence::enable();
		this->ErrorEntry::LoggerNameField::VTSequence::enable();
		this->ErrorEntry::MessageField::VTSequence::enable();
		this->ErrorEntry::ErrorCodeField::VTSequence::enable();
		this->ErrorEntry::LocationField::VTSequence::enable();
	}
}