export module Glas.Output;
export import Glas.StringOutputFormat;
export import std;


export namespace Glas
{
    template <typename T = std::vector<StringOutputFormat>>
    class Output {
    public:
        Output() = default;
        virtual ~Output() = default;
    private:
        Output(const Output&) = delete;
        Output& operator=(const Output&) = delete;
        Output(Output&&) noexcept = delete;
        Output& operator=(Output&&) noexcept = delete;
    public:
        virtual void output(const T& formatted) & = 0;
    };
}