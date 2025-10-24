export module Glas.StringOutputFormat;
export import Glas.VTStyle;
export import std;


export namespace Glas
{
	class StringOutputFormat {
	public:
		std::optional<std::string_view> type;
		std::optional<std::string> output;
		std::optional<std::string> vtBegin;
		std::optional<VTStyle> style;
	};
}