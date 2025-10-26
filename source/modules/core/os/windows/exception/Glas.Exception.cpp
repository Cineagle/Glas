module;
#include <windows.h>

module Glas.Exception;


namespace Glas
{
    Exception::Exception(const std::string_view customMessage, const std::source_location location) {
        std::string text{ "# Exception" };

        text.append("\n\n*Message:*\n`");
        text.append(customMessage);

        text.append(formError());
        text.append(formLocation(location));
        text.append(formTrace());

        description = std::move(text);
    }

    std::string Exception::formError() const & {
        const std::error_code code{ static_cast<int>(GetLastError()), std::system_category() };

        std::string text;

        text.append("\n\n*Last Error Code:*");

        text.append("\n- Value: ");
        text.append(std::to_string(code.value()));

        text.append("\n- Description: `");
        text.append(code.message());
        text.push_back('`');

        return text;
    }

    std::string Exception::formLocation(const std::source_location location) const & {
#ifdef NDEBUG 
        const std::string file{ std::filesystem::path(location.file_name()).filename().string() };
#else
        const std::string file{ location.file_name() };
#endif
        std::string text;

        text.append("\n\n*Location:*");

        text.append("\n- File: `");
        text.append(file);
        text.push_back('`');

        text.append("\n- Line: ");
        text.append(std::to_string(location.line()));

        text.append("\n- Column: ");
        text.append(std::to_string(location.column()));

        text.append("\n- Function: `");
        text.append(location.function_name());
        text.push_back('`');

        return text;
    }

    std::string Exception::formTrace() const & {
        std::string text;
        text.append("\n\n*StackTrace:*");

#ifdef NDEBUG
        text.append("\n-----------------------------------------------");
        text.append("\n`StackTrace not captured in Release build.`");
        text.append("\n-----------------------------------------------");
#else
        for (const auto& entry : std::stacktrace::current() | std::views::drop(1)) {
            text.append("\n-----------------------------------------------");

            text.append("\n- File: `");
            text.append(entry.source_file());

            text.append("`\n- Line: ");
            text.append(std::to_string(entry.source_line()));

            text.append("\n- Function: `");
            text.append(entry.description());
            text.push_back('`');
        }
        text.append("\n-----------------------------------------------");
#endif
        return text;
    }
}