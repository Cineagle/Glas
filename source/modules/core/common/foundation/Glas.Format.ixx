export module Glas.Format;
export import Glas.Exception;
export import std;


export namespace Glas
{
    class Format {
    public:
        Format(const std::string_view format, auto&&... args);
    private:
        Format() = delete;
        Format(const Format&) = delete;
        Format& operator=(const Format&) = delete;
        Format(Format&&) noexcept = delete;
        Format& operator=(Format&&) noexcept = delete;
    public:
        std::string text;
    };
}

export namespace Glas
{
    Format::Format(const std::string_view format, auto&&... args) {
        try {
            text = std::vformat(format, std::make_format_args(args...));
        }
        catch (const std::format_error& ex) {
            throw Exception{ std::format("Formatting error: {}.", ex.what()) };
        }
    }
}