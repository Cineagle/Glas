export module Glas.FileStringOutput;
export import Glas.File;
export import Glas.Output;
export import Glas.Concepts;
export import Glas.StringOutputFormat;
export import Glas.Exception;
export import std;


export namespace Glas
{
	class FileStringOutput final :
		public File,
		public Output<std::vector<StringOutputFormat>>
	{
	public:
		using File::PathType;
	public:
		virtual ~FileStringOutput() override;
	private:
		FileStringOutput(Glas::StringLike auto&& path, const PathType pathType,
			const std::size_t flushSize, const std::ios_base::openmode openMode);

		explicit FileStringOutput(const std::string_view id);

		FileStringOutput() = delete;
		FileStringOutput(const FileStringOutput&) = delete;
		FileStringOutput& operator=(const FileStringOutput&) = delete;
		FileStringOutput(FileStringOutput&&) noexcept = delete;
		FileStringOutput& operator=(FileStringOutput&&) noexcept = delete;
	public:
		static std::shared_ptr<FileStringOutput> create(Glas::StringLike auto&& path,
			const PathType pathType, const std::size_t flushSize = 0, 
			const std::ios_base::openmode openMode = 
			std::ios::out | std::ios::trunc | std::ios::binary);

		static std::shared_ptr<FileStringOutput> create(const std::string_view id);
	public:
		virtual void output(const std::vector<StringOutputFormat>& formatted) & override;
	private:
		void writeToFile() &;
	private:
		std::string buffer;
		std::mutex outputMutex;
	};
}

export namespace Glas
{
	FileStringOutput::FileStringOutput(Glas::StringLike auto&& path, const PathType pathType,
		const std::size_t flushSize, const std::ios_base::openmode openMode) :

		File{ std::forward<decltype(path)>(path), pathType, flushSize, openMode }
	{}

	FileStringOutput::FileStringOutput(const std::string_view id) :
		File{ id }
	{}

	FileStringOutput::~FileStringOutput()	{
		writeToFile();
	}

	std::shared_ptr<FileStringOutput> FileStringOutput::create(Glas::StringLike auto&& path,
		const PathType pathType, const std::size_t flushSize, const std::ios_base::openmode openMode)
	{
		auto ptr = std::unique_ptr<FileStringOutput>{
			new FileStringOutput(std::forward<decltype(path)>(path), pathType, flushSize, openMode)
		};
		return std::shared_ptr<FileStringOutput>(std::move(ptr));
	}

	std::shared_ptr<FileStringOutput> FileStringOutput::create(const std::string_view id)
	{
		auto ptr = std::unique_ptr<FileStringOutput>{
			new FileStringOutput(id)
		};
		return std::shared_ptr<FileStringOutput>(std::move(ptr));
	}

	void FileStringOutput::output(const std::vector<StringOutputFormat>& formatted)	& {
		const std::lock_guard lock{ outputMutex };

		for (const auto& item : formatted) {
			if (item.output && !item.output.value().empty()) {
				buffer.append(item.output.value());
			}
		}

		if (flushSize && buffer.size() >= flushSize) {
			writeToFile();
		}
	}

	void FileStringOutput::writeToFile() & {
		file.write(buffer.data(), buffer.size());
		buffer.clear();
	}
}