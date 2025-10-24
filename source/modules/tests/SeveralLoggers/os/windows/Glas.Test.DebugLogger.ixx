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
			Logger<
				SuccessEntry<
					TimePointField, TypeField, LoggerNameField, 
					MessageField
				>,
				WarningEntry<
					TimePointField, TypeField, LoggerNameField,
					MessageField
				>,
				ErrorEntry<
					TimePointField, TypeField, LoggerNameField,
					MessageField, ErrorCodeField,	LocationField
				>
			>
	{
	public:
		using Success = SuccessEntry<
			TimePointField, TypeField, LoggerNameField,
			MessageField
		>;
		using Warning = WarningEntry<
			TimePointField, TypeField, LoggerNameField,
			MessageField
		>;
		using Error = ErrorEntry<
			TimePointField, TypeField, LoggerNameField,
			MessageField, ErrorCodeField, LocationField
		>;
	private:
		DebugLogger(StringLike auto&& loggerName,
			const Queue::Scheme queueScheme,
			const std::size_t queueCapacity);
	public:
		static std::shared_ptr<DebugLogger> create(StringLike auto&& loggerName,
			const Queue::Scheme queueScheme = Queue::Scheme::Bound,
			const std::size_t queueCapacity = 1024);
	private:
		void tune() &;
		void tuneSuccessEntry() &;
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
	DebugLogger::DebugLogger(StringLike auto&& loggerName,
		const Queue::Scheme queueScheme, const std::size_t queueCapacity) :

			Logger<
				SuccessEntry<
					TimePointField, TypeField, LoggerNameField, 
					MessageField
				>,
				WarningEntry<
					TimePointField, TypeField, LoggerNameField,
					MessageField
				>,
				ErrorEntry<
					TimePointField, TypeField, LoggerNameField,
					MessageField, ErrorCodeField,	LocationField
				>
		>{ std::forward<decltype(loggerName)>(loggerName), queueScheme, queueCapacity }
	{
		tune();
	}

	std::shared_ptr<DebugLogger> DebugLogger::create(
		StringLike auto&& loggerName, const Queue::Scheme queueScheme,
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

		this->SuccessEntry::TimePointField::padding(Padding{ .spaceAfter{ 2 } });
		this->SuccessEntry::TypeField::padding(Padding{ .spaceAfter{ 2 } });
		this->SuccessEntry::LoggerNameField::padding(Padding{ .spaceAfter{ 2 } });
		this->SuccessEntry::MessageField::padding(Padding{ .breakAfter{ 1 } });

		this->SuccessEntry::TimePointField::VTSequence::enable();
		this->SuccessEntry::TypeField::VTSequence::enable();
		this->SuccessEntry::LoggerNameField::VTSequence::enable();
		this->SuccessEntry::MessageField::VTSequence::enable();

		this->SuccessEntry::scheme(Entry::Scheme::Direct);
	}

	void DebugLogger::tuneWarningEntry() & {
		this->WarningEntry::TimePointField::enable();
		this->WarningEntry::TypeField::enable();
		this->WarningEntry::LoggerNameField::enable();
		this->WarningEntry::MessageField::enable();

		this->WarningEntry::TimePointField::padding(Padding{ .spaceAfter{ 2 } });
		this->WarningEntry::TypeField::padding(Padding{ .spaceAfter{ 2 } });
		this->WarningEntry::LoggerNameField::padding(Padding{ .spaceAfter{ 2 } });
		this->WarningEntry::MessageField::padding(Padding{ .breakAfter{ 1 } });

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

		this->ErrorEntry::TimePointField::padding(Padding{ .spaceAfter{ 2 } });
		this->ErrorEntry::TypeField::padding(Padding{ .spaceAfter{ 2 } });
		this->ErrorEntry::LoggerNameField::padding(Padding{ .spaceAfter{ 2 } });
		this->ErrorEntry::MessageField::padding(Padding{ .breakAfter{ 1 } });
		this->ErrorEntry::ErrorCodeField::padding(Padding{ .breakAfter{ 1 } });
		this->ErrorEntry::LocationField::padding(Padding{ .breakAfter{ 1 } });

		this->ErrorEntry::TimePointField::VTSequence::enable();
		this->ErrorEntry::TypeField::VTSequence::enable();
		this->ErrorEntry::LoggerNameField::VTSequence::enable();
		this->ErrorEntry::MessageField::VTSequence::enable();
		this->ErrorEntry::ErrorCodeField::VTSequence::enable();
		this->ErrorEntry::LocationField::VTSequence::enable();
	}
}