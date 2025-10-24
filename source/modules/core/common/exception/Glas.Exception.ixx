export module Glas.Exception;
import std;


export namespace Glas
{
    class Exception :
        public std::exception
    {
    public:
        explicit Exception(const std::string_view customMessage,
            const std::source_location location = std::source_location::current());
    public:
        const char* what() const noexcept override;
    private:
        std::string formError() const;
        std::string formLocation(const std::source_location location) const;
        std::string formTrace() const;
    private:
        std::string description;
    };
}

export namespace Glas
{
    const char* Exception::what() const noexcept {
        return description.c_str();
    }
}