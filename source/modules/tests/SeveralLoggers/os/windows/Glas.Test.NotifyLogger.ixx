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
			Glas::Logger<
				Glas::InfoEntry<
					Glas::TimePointField, Glas::TypeField, Glas::LoggerNameField,
					Glas::MessageField
				>,
				Glas::SuccessEntry<
					Glas::TimePointField, Glas::TypeField, Glas::LoggerNameField,
					Glas::MessageField
				>
			>
	{
	public:
		using Info = Glas::InfoEntry<
			Glas::TimePointField, Glas::TypeField, Glas::LoggerNameField,
			Glas::MessageField
		>;
		using Success = Glas::SuccessEntry<
			Glas::TimePointField, Glas::TypeField, Glas::LoggerNameField,
			Glas::MessageField
		>;
	private:
		NotifyLogger(StringLike auto&& loggerName,
			const Glas::Queue::Scheme queueScheme,
			const std::size_t queueCapacity);
	public:
		static std::shared_ptr<NotifyLogger> create(StringLike auto&& loggerName,
			const Glas::Queue::Scheme queueScheme = Glas::Queue::Scheme::Bound,
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
		const Glas::Queue::Scheme queueScheme, const std::size_t queueCapacity) :

			Glas::Logger<
				Glas::InfoEntry<
					Glas::TimePointField, Glas::TypeField, Glas::LoggerNameField,
					Glas::MessageField
				>,
				Glas::SuccessEntry<
					Glas::TimePointField, Glas::TypeField, Glas::LoggerNameField,
					Glas::MessageField
				>
			>{ std::forward<decltype(loggerName)>(loggerName), queueScheme, queueCapacity }
	{
		tune();
	}

	std::shared_ptr<NotifyLogger> NotifyLogger::create(
		StringLike auto&& loggerName, const Glas::Queue::Scheme queueScheme,
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

		this->InfoEntry::TimePointField::padding(Glas::Padding{ .spaceAfter{ 2 } });
		this->InfoEntry::TypeField::padding(Glas::Padding{ .spaceAfter{ 2 } });
		this->InfoEntry::LoggerNameField::padding(Glas::Padding{ .spaceAfter{ 2 } });
		this->InfoEntry::MessageField::padding(Glas::Padding{ .breakAfter{ 1 } });
	}

	void NotifyLogger::tuneSuccessEntry() & {
		this->SuccessEntry::TimePointField::enable();
		this->SuccessEntry::TypeField::enable();
		this->SuccessEntry::LoggerNameField::enable();
		this->SuccessEntry::MessageField::enable();

		this->SuccessEntry::TimePointField::padding(Glas::Padding{ .spaceAfter{ 2 } });
		this->SuccessEntry::TypeField::padding(Glas::Padding{ .spaceAfter{ 2 } });
		this->SuccessEntry::LoggerNameField::padding(Glas::Padding{ .spaceAfter{ 2 } });
		this->SuccessEntry::MessageField::padding(Glas::Padding{ .breakAfter{ 1 } });
	}
}