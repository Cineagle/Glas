export module Glas.Test.BinaryOutputFile;
export import Glas.File;
export import Glas.Output;
export import Glas.Concepts;
export import Glas.StringOutputFormat;
export import Glas.Exception;
export import std;


export namespace Glas::Test
{
	class BinaryOutputFile final :
		public Glas::File,
		public Glas::Output<std::vector<std::byte>>
	{
	public:
		using Glas::File::PathType;
	public:
		virtual ~BinaryOutputFile() override;
	private:
		BinaryOutputFile(StringLike auto&& path, const PathType pathType,
			const std::size_t flushSize, const std::ios_base::openmode openMode);

		explicit BinaryOutputFile(const std::string_view id);

		BinaryOutputFile() = delete;
		BinaryOutputFile(const BinaryOutputFile&) = delete;
		BinaryOutputFile& operator=(const BinaryOutputFile&) = delete;
		BinaryOutputFile(BinaryOutputFile&&) noexcept = delete;
		BinaryOutputFile& operator=(BinaryOutputFile&&) noexcept = delete;
	public:
		static std::shared_ptr<BinaryOutputFile> create(StringLike auto&& path,
			const PathType pathType, const std::size_t flushSize = 0,
			const std::ios_base::openmode openMode =
			std::ios::out | std::ios::trunc | std::ios::binary);

		static std::shared_ptr<BinaryOutputFile> create(const std::string_view id);
	public:
		virtual void output(const std::vector<std::byte>& formatted) & override;
	private:
		void writeToFile() &;
	private:
		std::vector<std::byte> buffer;
		std::mutex outputMutex;
	};
}

export namespace Glas::Test
{
	BinaryOutputFile::BinaryOutputFile(StringLike auto&& path, const PathType pathType,
		const std::size_t flushSize, const std::ios_base::openmode openMode) :

		Glas::File{ std::forward<decltype(path)>(path), pathType, flushSize, openMode }
	{}

	BinaryOutputFile::BinaryOutputFile(const std::string_view id) :
		Glas::File{ id }
	{}

	BinaryOutputFile::~BinaryOutputFile() {
		writeToFile();
	}

	std::shared_ptr<BinaryOutputFile> BinaryOutputFile::create(StringLike auto&& path,
		const PathType pathType, const std::size_t flushSize, const std::ios_base::openmode openMode)
	{
		auto ptr = std::unique_ptr<BinaryOutputFile>{
			new BinaryOutputFile(std::forward<decltype(path)>(path), pathType, flushSize, openMode)
		};
		return std::shared_ptr<BinaryOutputFile>(std::move(ptr));
	}

	std::shared_ptr<BinaryOutputFile> BinaryOutputFile::create(const std::string_view id)
	{
		auto ptr = std::unique_ptr<BinaryOutputFile>{
			new BinaryOutputFile(id)
		};
		return std::shared_ptr<BinaryOutputFile>(std::move(ptr));
	}

	void BinaryOutputFile::output(const std::vector<std::byte>& formatted) & {
		const std::lock_guard lock{ outputMutex };

		buffer.append_range(formatted);

		if (flushSize && buffer.size() >= flushSize) {
			writeToFile();
		}
	}

	void BinaryOutputFile::writeToFile() & {
		file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
		buffer.clear();
	}
}