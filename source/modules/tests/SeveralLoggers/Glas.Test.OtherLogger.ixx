export module Glas.Test.OtherLogger;
export import Glas.Logger;

export import Glas.InfoEntry;

export import Glas.TimePointField;
export import Glas.MessageField;
export import Glas.LoggerNameField;

export import Glas.FileStringOutput;

export import std;


export namespace Glas::Test
{
	class OtherLogger :
		public
			Logger<
				InfoEntry<
					TimePointField, LoggerNameField, MessageField
				>
			>
	{
	public:
		using Info = InfoEntry<
			TimePointField, LoggerNameField, MessageField
		>;
	private:
		OtherLogger(StringLike auto&& loggerName,
			const Queue::Scheme queueScheme,
			const std::size_t queueCapacity);
	public:
		static std::shared_ptr<OtherLogger> create(StringLike auto&& loggerName,
			const Queue::Scheme queueScheme = Queue::Scheme::Bound,
			const std::size_t queueCapacity = 1024);
	private:
		void createOutput() &;
		void tune() &;
		void tuneInfoEntry() &;
	private:
		std::shared_ptr<Output<>> file;
	};
}

export namespace Glas::Test
{
	OtherLogger::OtherLogger(StringLike auto&& loggerName,
		const Queue::Scheme queueScheme, const std::size_t queueCapacity) :

		Logger<
			InfoEntry<
				TimePointField, LoggerNameField, MessageField
			>
		>{ std::forward<decltype(loggerName)>(loggerName), queueScheme, queueCapacity }
	{
		createOutput();
		tune();
	}

	std::shared_ptr<OtherLogger> OtherLogger::create(
		StringLike auto&& loggerName, const Queue::Scheme queueScheme,
		const std::size_t queueCapacity)
	{
		auto ptr = std::unique_ptr<OtherLogger>{
			new OtherLogger(std::forward<decltype(loggerName)>(loggerName), queueScheme,
				queueCapacity)
		};
		return std::shared_ptr<OtherLogger>(std::move(ptr));
	}

	void OtherLogger::createOutput() & {
		file = FileStringOutput::create("OtherLogger.txt",
			FileStringOutput::PathType::Relative);
	}

	void OtherLogger::tune() & {
		tuneInfoEntry();
	}

	void OtherLogger::tuneInfoEntry() & {
		this->InfoEntry::TimePointField::enable();
		this->InfoEntry::LoggerNameField::enable();
		this->InfoEntry::MessageField::enable();

		this->InfoEntry::TimePointField::padding(Padding{ .spaceAfter{ 2 } });
		this->InfoEntry::LoggerNameField::padding(Padding{ .spaceAfter{ 2 } });
		this->InfoEntry::MessageField::padding(Padding{ .breakAfter{ 1 } });

		this->InfoEntry::outputs(file);
	}
}