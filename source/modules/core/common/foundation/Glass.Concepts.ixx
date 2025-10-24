export module Glas.Concepts;
export import std;


export namespace Glas
{
    template <typename T, typename CharT>
    concept StringLikeT =
        std::convertible_to<T, std::basic_string_view<CharT>> ||
        std::convertible_to<T, std::basic_string<CharT>>;

    template <typename T>
    concept StringLike =
        std::convertible_to<T, std::string_view> ||
        std::convertible_to<T, std::string>;

    template <typename T>
    concept WStringLike =
        std::convertible_to<T, std::wstring_view> ||
        std::convertible_to<T, std::wstring>;
}