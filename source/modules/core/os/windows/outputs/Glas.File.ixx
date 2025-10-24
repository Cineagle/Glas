module;
#include <windows.h>

export module Glas.File;
export import Glas.Concepts;
export import Glas.Exception;
export import std;


export namespace Glas
{
	class File {
	public:
		enum class PathType { Relative, Temp };
	public:
		virtual ~File();
	public:
		File(Glas::StringLike auto&& path, const PathType pathType,
			const std::size_t flushSize, const std::ios_base::openmode openMode);

		explicit File(const std::string_view id);
	private:
		File() = delete;
		File(const File&) = delete;
		File& operator=(const File&) = delete;
		File(File&&) noexcept = delete;
		File& operator=(File&&) noexcept = delete;
	private:
		static std::string toMultiByte(const std::wstring_view text);
	private:
		void openFile() &;
		void parseCmdConfig(const std::string_view id) &;
		void parseCmdTokens() &;
		void parsePath(const std::string_view id, const std::string_view token) &;
		void parsePathType(const std::string_view id, const std::string_view token) &;
		void parseFlushSize(const std::string_view id, const std::string_view token) &;
		void parseOpenMode(const std::string_view id, const std::string_view token) &;
	private:
		inline static std::mutex pathMutex;
		inline static std::unordered_set<std::string> paths;
	protected:
		std::ofstream file;
		std::size_t flushSize{};
	private:
		std::string path;
		PathType pathType{ PathType::Relative };
		std::ios_base::openmode openMode{};
		std::vector<std::string> tokens;
		std::string finalPath;
	};
}

export namespace Glas
{
	File::File(Glas::StringLike auto&& path, const PathType pathType,
		const std::size_t flushSize, const std::ios_base::openmode openMode) :

		path{ std::forward<decltype(path)>(path) },
		pathType{ pathType },
		flushSize{ flushSize },
		openMode{ openMode }
	{
		if (this->path.empty()) {
			throw Exception{ "`path` is empty." };
		}
		openFile();
	}

	File::File(const std::string_view id) {
		parseCmdConfig(id);
		openFile();
	}

	File::~File() {
		const std::lock_guard lock{ pathMutex };
		paths.erase(finalPath);
	}

	void File::openFile() & {
		std::filesystem::path systemPath{ path };

		if (!systemPath.is_absolute()) {
			switch (pathType) {
			case PathType::Relative:
				systemPath = std::filesystem::current_path() / path;
				break;
			case PathType::Temp:
				systemPath = std::filesystem::temp_directory_path() / path;
				break;
			}
		}

		if (!systemPath.has_filename()) {
			throw Exception{ "Invalid `path` (missing filename)." };
		}

		if (systemPath.has_parent_path()) {
			try {
				std::filesystem::create_directories(systemPath.parent_path());
			}
			catch (const std::filesystem::filesystem_error& e) {
				throw Exception{ "Cannot create directory: " + systemPath.parent_path().string() +
								 " reason: " + e.what() };
			}
		}

		const std::lock_guard lock{ pathMutex };
		finalPath = systemPath.string();
		if (paths.contains(finalPath)) {
			throw Exception{ "`path` is already in use." };
		}
		paths.emplace(finalPath);

		file.open(systemPath, openMode);
		if (!file.is_open()) {
			throw Exception{ std::format("Failed to open `file` for writing. `path`: {}.", path) };
		}
	}

	void File::parseCmdConfig(const std::string_view id) & {
		if (id.empty()) {
			throw Exception{ "`id` is empty." };
		}

		parseCmdTokens();

		for (const auto& token : tokens | std::views::drop(1)) {
			parsePath(id, token);
			parsePathType(id, token);
			parseFlushSize(id, token);
			parseOpenMode(id, token);
		}
	}

	void File::parseCmdTokens() & {
		int count{};
		const auto args = CommandLineToArgvW(GetCommandLineW(), &count);

		if (!args) {
			throw Exception{ "`CommandLineToArgvW` failed." };
		}

		for (int i{}; i < count; ++i) {
			tokens.push_back(toMultiByte(args[i]));
		}

		if (LocalFree(args)) {
			throw Exception{ "`LocalFree` failed." };
		}
	}

	std::string File::toMultiByte(const std::wstring_view text) {
		if (text.empty()) {
			throw Exception{ "`text` is empty." };
		}

		const auto size = WideCharToMultiByte(CP_UTF8, 0, text.data(),
			static_cast<int>(text.size()), nullptr, 0, nullptr, nullptr);

		if (!size) {
			throw Exception{ "`WideCharToMultiByte` could not calculate buffer size." };
		}

		std::string converted(size, '\0');
		if (!WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()),
			converted.data(), size, nullptr, nullptr))
		{
			throw Exception{ "`WideCharToMultiByte` conversion failed." };
		}

		return converted;
	}

	void File::parsePath(const std::string_view id, const std::string_view token) & {
		const auto key = "--" + std::string{ id } + ".path=";
		if (!token.contains(key)) {
			return;
		}

		const auto value = token.substr(key.size(), token.size() - key.size());
		if (value.empty()) {
			throw Exception{ std::format("`path` is empty for `id`: {}. `token`: {}.", id, token) };
		}

		path = value;
	}

	void File::parsePathType(const std::string_view id, const std::string_view token) & {
		const auto key = "--" + std::string{ id } + ".pathType=";
		if (!token.contains(key)) {
			return;
		}

		const auto value = token.substr(key.size(), token.size() - key.size());
		if (value.empty()) {
			throw Exception{ std::format("`pathType` is empty for `id`: {}. `token`: {}.", id,
				token) };
		}

		if (value.contains("Relative")) {
			pathType = PathType::Relative;
		}
		else if (value.contains("Temp")) {
			pathType = PathType::Temp;
		}
		else {
			throw Exception{ std::format("Invalid `pathType` value: {}. For `id`: {}. `token`: {}.",
				value, id, token) };
		}
	}

	void File::parseFlushSize(const std::string_view id, const std::string_view token) & {
		const auto key = "--" + std::string{ id } + ".flushSize=";
		if (!token.contains(key)) {
			return;
		}

		const auto value = token.substr(key.size(), token.size() - key.size());
		if (value.empty()) {
			throw Exception{ std::format("`flushSize` is empty for `id`: {}. `token`: {}.", id,
				token) };
		}

		const auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), flushSize);

		if (ec != std::errc{}) {
			throw Exception{ std::format("Invalid `flushSize` value: {}. For `id`: {}. `token`: {}.",
				value, id, token) };
		}
	}

	void File::parseOpenMode(const std::string_view id, const std::string_view token) & {
		const auto key = "--" + std::string{ id } + ".openMode=";
		if (!token.contains(key)) {
			return;
		}

		const auto value = token.substr(key.size(), token.size() - key.size());
		if (value.empty()) {
			throw Exception{ std::format("`openMode` is empty for `id`: {}. `token`: {}.", id, token) };
		}

		static constexpr std::array<std::pair<std::string_view, std::ios_base::openmode>, 7> pairs{ {
			{ "app", std::ios::app },
			{ "binary", std::ios::binary },
			{ "in", std::ios::in },
			{ "out", std::ios::out },
			{ "trunc", std::ios::trunc },
			{ "ate", std::ios::ate },
			{ "noreplace", std::ios::noreplace }
		} };

		for (const auto& pair : pairs) {
			if (value.contains(pair.first)) {
				openMode |= pair.second;
			}
		}

		if (!openMode) {
			throw Exception{ std::format("Invalid `openMode` value: {}. For `id`: {}. `token`: {}.",
				value, id, token) };
		}
	}
}