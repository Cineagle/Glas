export module Glas.Test.NotifyLogger;
export import Glas.Logger;

export import Glas.InfoEntry;
export import Glas.SuccessEntry;

export import Glas.TimePointField;
export import Glas.TypeField;
export import Glas.LoggerNameField;
export import Glas.MessageField;

export import std;


export namespace Glas::Test
{
	class NotifyLogger :
		public
			Logger<
				InfoEntry<
					TimePointField, TypeField, LoggerNameField,
					MessageField
				>,
				SuccessEntry<
					TimePointField, TypeField, LoggerNameField,
					MessageField
				>
			>
	{
	public:
		using Info = InfoEntry<
			TimePointField, TypeField, LoggerNameField,
			MessageField
		>;
		using Success = SuccessEntry<
			TimePointField, TypeField, LoggerNameField,
			MessageField
		>;
	private:
		NotifyLogger(StringLike auto&& loggerName,
			const Queue::Scheme queueScheme,
			const std::size_t queueCapacity);
	public:
		static std::shared_ptr<NotifyLogger> create(StringLike auto&& loggerName,
			const Queue::Scheme queueScheme = Queue::Scheme::Bound,
			const std::size_t queueCapacity = 1024);
	private:
		void tune() &;
		void tuneInfoEntry() &;
		void tuneSuccessEntry() &;
	};
}

export namespace Glas::Test
{
	NotifyLogger::NotifyLogger(StringLike auto&& loggerName,
		const Queue::Scheme queueScheme, const std::size_t queueCapacity) :

			Logger<
				InfoEntry<
					TimePointField, TypeField, LoggerNameField,
					MessageField
				>,
				SuccessEntry<
					TimePointField, TypeField, LoggerNameField,
					MessageField
				>
			>{ std::forward<decltype(loggerName)>(loggerName), queueScheme, queueCapacity }
	{
		tune();
	}

	std::shared_ptr<NotifyLogger> NotifyLogger::create(
		StringLike auto&& loggerName, const Queue::Scheme queueScheme,
		const std::size_t queueCapacity)
	{
		auto ptr = std::unique_ptr<NotifyLogger>{
			new NotifyLogger(std::forward<decltype(loggerName)>(loggerName), queueScheme,
				queueCapacity)
		};
		return std::shared_ptr<NotifyLogger>(std::move(ptr));
	}

	void NotifyLogger::tune() & {
		tuneInfoEntry();
		tuneSuccessEntry();
	}

	void NotifyLogger::tuneInfoEntry() & {
		this->InfoEntry::TimePointField::enable();
		this->InfoEntry::TypeField::enable();
		this->InfoEntry::LoggerNameField::enable();
		this->InfoEntry::MessageField::enable();

		this->InfoEntry::TimePointField::padding(Padding{ .spaceAfter{ 2 } });
		this->InfoEntry::TypeField::padding(Padding{ .spaceAfter{ 2 } });
		this->InfoEntry::LoggerNameField::padding(Padding{ .spaceAfter{ 2 } });
		this->InfoEntry::MessageField::padding(Padding{ .breakAfter{ 1 } });
	}

	void NotifyLogger::tuneSuccessEntry() & {
		this->SuccessEntry::TimePointField::enable();
		this->SuccessEntry::TypeField::enable();
		this->SuccessEntry::LoggerNameField::enable();
		this->SuccessEntry::MessageField::enable();

		this->SuccessEntry::TimePointField::padding(Padding{ .spaceAfter{ 2 } });
		this->SuccessEntry::TypeField::padding(Padding{ .spaceAfter{ 2 } });
		this->SuccessEntry::LoggerNameField::padding(Padding{ .spaceAfter{ 2 } });
		this->SuccessEntry::MessageField::padding(Padding{ .breakAfter{ 1 } });
	}
}