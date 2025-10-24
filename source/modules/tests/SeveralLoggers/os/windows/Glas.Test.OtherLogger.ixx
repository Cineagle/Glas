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
			Glas::Logger<
				Glas::InfoEntry<
					Glas::TimePointField, Glas::LoggerNameField, Glas::MessageField
				>
			>
	{
	public:
		using Info = Glas::InfoEntry<
			Glas::TimePointField, Glas::LoggerNameField, Glas::MessageField
		>;
	private:
		OtherLogger(Glas::StringLike auto&& loggerName,
			const Glas::Queue::Scheme queueScheme,
			const std::size_t queueCapacity);
	public:
		static std::shared_ptr<OtherLogger> create(Glas::StringLike auto&& loggerName,
			const Glas::Queue::Scheme queueScheme = Glas::Queue::Scheme::Bound,
			const std::size_t queueCapacity = 1024);
	private:
		void createOutput() &;
		void tune() &;
		void tuneInfoEntry() &;
	private:
		std::shared_ptr<Glas::Output<>> file;
	};
}

export namespace Glas::Test
{
	OtherLogger::OtherLogger(Glas::StringLike auto&& loggerName,
		const Glas::Queue::Scheme queueScheme, const std::size_t queueCapacity) :

		Glas::Logger<
			Glas::InfoEntry<
				Glas::TimePointField, Glas::LoggerNameField, Glas::MessageField
			>
		>{ std::forward<decltype(loggerName)>(loggerName), queueScheme, queueCapacity }
	{
		createOutput();
		tune();
	}

	std::shared_ptr<OtherLogger> OtherLogger::create(
		Glas::StringLike auto&& loggerName, const Glas::Queue::Scheme queueScheme,
		const std::size_t queueCapacity)
	{
		auto ptr = std::unique_ptr<OtherLogger>{
			new OtherLogger(std::forward<decltype(loggerName)>(loggerName), queueScheme,
				queueCapacity)
		};
		return std::shared_ptr<OtherLogger>(std::move(ptr));
	}

	void OtherLogger::createOutput() & {
		file = Glas::FileStringOutput::create("OtherLogger.txt",
			Glas::FileStringOutput::PathType::Relative);
	}

	void OtherLogger::tune() & {
		tuneInfoEntry();
	}

	void OtherLogger::tuneInfoEntry() & {
		this->InfoEntry::TimePointField::enable();
		this->InfoEntry::LoggerNameField::enable();
		this->InfoEntry::MessageField::enable();

		this->InfoEntry::TimePointField::padding(Glas::Padding{ .spaceAfter{ 2 } });
		this->InfoEntry::LoggerNameField::padding(Glas::Padding{ .spaceAfter{ 2 } });
		this->InfoEntry::MessageField::padding(Glas::Padding{ .breakAfter{ 1 } });

		this->InfoEntry::outputs(file);
	}
}