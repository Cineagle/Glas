module;
#include <windows.h>

export module Glas.Test.WindowsEventOutput;
export import Glas.Output;
export import Glas.StringOutputFormat;
export import Glas.Exception;
export import std;


export namespace Glas::Test
{
	class WindowsEventOutput final :
		public Glas::Output<std::vector<Glas::StringOutputFormat>>
	{
	public:
		virtual ~WindowsEventOutput() override;
	private:
		WindowsEventOutput();

		WindowsEventOutput(const WindowsEventOutput&) = delete;
		WindowsEventOutput& operator=(const WindowsEventOutput&) = delete;
		WindowsEventOutput(WindowsEventOutput&&) noexcept = delete;
		WindowsEventOutput& operator=(WindowsEventOutput&&) noexcept = delete;
	public:
		static std::shared_ptr<WindowsEventOutput> create();
	public:
		virtual void output(const std::vector<Glas::StringOutputFormat>& formatted) & override;
	private:
		HANDLE event{};
		std::mutex outputMutex;
	};
}

export namespace Glas::Test
{
	WindowsEventOutput::WindowsEventOutput() {
		event = RegisterEventSourceA(NULL, "CE::Test::Logger");
		if (!event) {
			throw Exception{ "`RegisterEventSourceA` failed." };
		}
	}

	WindowsEventOutput::~WindowsEventOutput() {
		if (event) {
			DeregisterEventSource(event);
		}
	}

	std::shared_ptr<WindowsEventOutput> WindowsEventOutput::create() {
		auto ptr = std::unique_ptr<WindowsEventOutput>{
			new WindowsEventOutput()
		};
		return std::shared_ptr<WindowsEventOutput>(std::move(ptr));
	}

	void WindowsEventOutput::output(const std::vector<Glas::StringOutputFormat>& formatted) & {
		const std::lock_guard lock{ outputMutex };

		for (const auto& item : formatted) {
			if (item.output && !item.output.value().empty()) {
				LPCSTR msgs[1]{ item.output.value().c_str() };
				
				if (item.type) {
					if (item.type.value().compare("Info") == 0) {
						ReportEventA(event, EVENTLOG_INFORMATION_TYPE, 0, 0, NULL, 1, 0,
							msgs, NULL);
					}
					else if (item.type.value().compare("Error") == 0) {
						ReportEventA(event, EVENTLOG_ERROR_TYPE, 0, 0, NULL, 1, 0,
							msgs, NULL);
					}
					else {
						ReportEventA(event, EVENTLOG_INFORMATION_TYPE, 0, 0, NULL, 1, 0,
							msgs, NULL);
					}
				}
			}
		}
	}
}