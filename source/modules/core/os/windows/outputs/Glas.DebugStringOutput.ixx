module;
#include <windows.h>

export module Glas.DebugStringOutput;
export import Glas.Output;
export import Glas.StringOutputFormat;
export import Glas.Exception;
export import std;


export namespace Glas
{
	class DebugStringOutput final :
		public Output<std::vector<StringOutputFormat>>
	{
	public:
		virtual ~DebugStringOutput() override;
	private:
		DebugStringOutput() = default;

		DebugStringOutput(const DebugStringOutput&) = delete;
		DebugStringOutput& operator=(const DebugStringOutput&) = delete;
		DebugStringOutput(DebugStringOutput&&) noexcept = delete;
		DebugStringOutput& operator=(DebugStringOutput&&) noexcept = delete;
	public:
		static std::shared_ptr<DebugStringOutput> create();
	public:
		virtual void output(const std::vector<StringOutputFormat>& formatted) & override;
	private:
		inline static std::mutex createdMutex;
		inline static bool created{};
	private:
		std::mutex outputMutex;
	};
}

export namespace Glas
{
	DebugStringOutput::~DebugStringOutput() {
		const std::lock_guard lock{ createdMutex };
		created = false;
	}

	std::shared_ptr<DebugStringOutput> DebugStringOutput::create() {
		const std::lock_guard lock{ createdMutex };

		if (created) {
			throw Exception{ "`DebugStringOutput` class accept only one instance." };
		}

		auto ptr = std::unique_ptr<DebugStringOutput>{
			new DebugStringOutput()
		};
		created = true;

		return std::shared_ptr<DebugStringOutput>(std::move(ptr));
	}

	void DebugStringOutput::output(const std::vector<StringOutputFormat>& formatted) & {
		const std::lock_guard lock{ outputMutex };

		for (const auto& item : formatted) {
			if (item.output && !item.output.value().empty()) {
				OutputDebugStringA(item.output.value().c_str());
			}
		}
	}
}