module;
#include <windows.h>
#include <stdio.h>

export module Glas.ConsoleStringOutput;
export import Glas.Output;
export import Glas.BasicCharBufferWriter;
export import Glas.StringOutputFormat;
export import Glas.VTSequence;
export import Glas.Concepts;
export import Glas.Exception;
export import std;


export namespace Glas
{
	class ConsoleDesc final {
	public:
		std::wstring consoleTitle{ L"Console" };
		std::wstring fontFaceName{ L"Consolas" };
		unsigned int fontSizeX{ 20 };
		unsigned int fontSizeY{ 20 };
		unsigned int fontWeight{ 500 };
		bool vtEnabled{};
	};


	class ConsoleStringOutput final :
		public Output<std::vector<StringOutputFormat>>
	{
	public:
		virtual ~ConsoleStringOutput() override;
	private:
		explicit ConsoleStringOutput(ForwardedAs<ConsoleDesc> auto&& desc);

		ConsoleStringOutput() = delete;
		ConsoleStringOutput(const ConsoleStringOutput&) = delete;
		ConsoleStringOutput& operator=(const ConsoleStringOutput&) = delete;
		ConsoleStringOutput(ConsoleStringOutput&&) noexcept = delete;
		ConsoleStringOutput& operator=(ConsoleStringOutput&&) noexcept = delete;
	public:
		static std::shared_ptr<ConsoleStringOutput> create(ForwardedAs<ConsoleDesc> auto&& desc);
	public:
		virtual void output(const std::vector<StringOutputFormat>& formatted) & override;
	private:
		void initConsole() &;

		void allocateConsole() &;
		void initFont() &;
		void initScreenBuffer() &;
		void initTitle() &;
		void enableVTMode() &;

		void shutdownConsole() &;

		void writeConsole(const StringOutputFormat& formatted) const &;
	private:
		inline static std::mutex createdMutex;
		inline static bool created{};

		static constexpr std::string_view vtEnd{ "\x1b[0m" };
	private:
		ConsoleDesc desc;

		CONSOLE_FONT_INFOEX fontInfoOrig{};
		CONSOLE_SCREEN_BUFFER_INFO bufferInfoOrig{};
		std::wstring titleOrig;
		unsigned long modeOrig{};

		HANDLE outHandle{};

		bool consoleAllocated{};
		bool consoleInitialized{};

		std::mutex outputMutex;
	};
}

export namespace Glas
{
	ConsoleStringOutput::ConsoleStringOutput(ForwardedAs<ConsoleDesc> auto&& desc) :
		desc{ std::forward<decltype(desc)>(desc) }
	{
		if (this->desc.fontFaceName.empty()) {
			throw Exception{ "`desc.fontFaceName` length is empty." };
		}
		if (this->desc.fontFaceName.length() > LF_FACESIZE) {
			throw Exception{ "`desc.fontFaceName` length > LF_FACESIZE." };
		}
		initConsole();
	}

	ConsoleStringOutput::~ConsoleStringOutput() {
		shutdownConsole();

		const std::lock_guard lock{ createdMutex };
		created = false;
	}

	std::shared_ptr<ConsoleStringOutput> ConsoleStringOutput::create(
		ForwardedAs<ConsoleDesc> auto&& desc) 
	{
		const std::lock_guard lock{ createdMutex };

		if (created) {
			throw Exception{ "`ConsoleStringOutput` class accept only one instance." };
		}

		auto ptr = std::unique_ptr<ConsoleStringOutput>{
			new ConsoleStringOutput(std::forward<decltype(desc)>(desc))
		};
		created = true;

		return std::shared_ptr<ConsoleStringOutput>(std::move(ptr));
	}

	void ConsoleStringOutput::initConsole() & {
		allocateConsole();
		initFont();
		initScreenBuffer();
		initTitle();
		enableVTMode();

		consoleInitialized = true;
	}

	void ConsoleStringOutput::allocateConsole() & {
		if (!GetConsoleWindow() && !AttachConsole(ATTACH_PARENT_PROCESS)) {
			if (!AllocConsole()) {
				throw Exception{ "`AllocConsole` failed." };
			}
			consoleAllocated = true;
		}

		outHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		if (!outHandle || outHandle == INVALID_HANDLE_VALUE) {
			throw Exception{ "`GetStdHandle` for STD_OUTPUT_HANDLE failed." };
		}
	}

	void ConsoleStringOutput::initFont() & {
		CONSOLE_FONT_INFOEX info{};
		info.cbSize = sizeof(CONSOLE_FONT_INFOEX);
		if (!GetCurrentConsoleFontEx(outHandle, FALSE, &info)) {
			throw Exception{ "`GetCurrentConsoleFontEx` failed." };
		}

		fontInfoOrig = info;
		info.dwFontSize.X = desc.fontSizeX;
		info.dwFontSize.Y = desc.fontSizeY;
		info.FontWeight = desc.fontWeight;

		Glas::WCharBufferWriter writer{ info.FaceName };

		const auto result = writer.assign(desc.fontFaceName);
		if (result != Glas::WCharBufferWriter::Result::Success) {
			throw Exception{ "`WCharBufferWriter` failed." };
		}

		if (!SetCurrentConsoleFontEx(outHandle, FALSE, &info)) {
			throw Exception{ "`SetCurrentConsoleFontEx` failed." };
		}
	}

	void ConsoleStringOutput::initScreenBuffer() & {
		CONSOLE_SCREEN_BUFFER_INFO info{};
		if (!GetConsoleScreenBufferInfo(outHandle, &info)) {
			throw Exception{ "`GetConsoleScreenBufferInfo` failed." };
		}

		bufferInfoOrig = info;
		info.srWindow.Right = static_cast<short>(info.srWindow.Right / 1.2f);
		info.srWindow.Bottom = static_cast<short>(info.srWindow.Bottom / 1.4f);

		if (!SetConsoleWindowInfo(outHandle, TRUE, &info.srWindow)) {
			throw Exception{ "`SetConsoleWindowInfo` failed." };
		}
	}

	void ConsoleStringOutput::initTitle() & {
		wchar_t title[MAX_PATH];
		if (!GetConsoleTitleW(title, MAX_PATH)) {
			throw Exception{ "`GetConsoleTitleW` failed." };
		}

		titleOrig = title;

		if (!SetConsoleTitleW(desc.consoleTitle.c_str())) {
			throw Exception{ "`SetConsoleTitleW` failed." };
		}
	}
	
	void ConsoleStringOutput::enableVTMode() & {
		if (desc.vtEnabled) {
			if (!GetConsoleMode(outHandle, &modeOrig)) {
				throw Exception{ "`GetConsoleMode` failed." };
			}
			if (!SetConsoleMode(outHandle, modeOrig | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
				throw std::runtime_error("`SetConsoleMode` failed");
			}
		}
	}

	void ConsoleStringOutput::shutdownConsole() & {
		if (consoleInitialized) {
			if (consoleAllocated) {
				const auto handle = GetConsoleWindow();
				if (!handle) {
					throw Exception{ "`handle` is NULL." };
				}
				if (!PostMessageW(handle, WM_CLOSE, 0, 0)) {
					throw Exception{ "`PostMessageW` failed." };
				}
				if (!FreeConsole()) {
					throw Exception{ "`FreeConsole` failed." };
				}
			}
			else {
				if (!SetCurrentConsoleFontEx(outHandle, FALSE, &fontInfoOrig)) {
					throw Exception{ "`SetCurrentConsoleFontEx` failed." };
				}
				if (!SetConsoleWindowInfo(outHandle, TRUE, &bufferInfoOrig.srWindow)) {
					throw Exception{ "`SetConsoleWindowInfo` failed." };
				}
				if (!SetConsoleTitleW(titleOrig.c_str())) {
					throw Exception{ "`SetConsoleTitleW` failed." };
				}
				if (!SetConsoleMode(outHandle, modeOrig)) {
					throw std::runtime_error("`SetConsoleMode` failed");
				}
			}
		}
	}

	void ConsoleStringOutput::output(const std::vector<StringOutputFormat>& formatted) & {
		const std::lock_guard lock{ outputMutex };

		for (const auto& item : formatted) {
			if (item.output && !item.output.value().empty()) {
				writeConsole(item);
			}
		}
	}

	void ConsoleStringOutput::writeConsole(const StringOutputFormat& formatted) const & {
		if (formatted.vtBegin) {
			if (!WriteConsoleA(outHandle, formatted.vtBegin.value().data(), 
				static_cast<unsigned int>(formatted.vtBegin.value().size()), nullptr, nullptr))
			{
				throw Exception{ "`WriteConsoleA` for the `vtSequence.begin` failed." };
			}
		}

		if (!WriteConsoleA(outHandle, formatted.output.value().data(), 
			static_cast<unsigned int>(formatted.output.value().size()), nullptr, nullptr))
		{
			throw Exception{ "`WriteConsoleA` for the `output` failed." };
		}

		if (formatted.vtBegin) {
			if (!WriteConsoleA(outHandle, vtEnd.data(),
				static_cast<unsigned int>(vtEnd.size()), nullptr, nullptr))
			{
				throw Exception{ "`WriteConsoleA` for the `vtSequence.end` failed." };
			}
		}
	}
}