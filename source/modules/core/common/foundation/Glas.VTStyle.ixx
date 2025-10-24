export module Glas.VTStyle;


export namespace Glas
{
    class VTStyle {
    public:
        class Color {
        public:
            unsigned char red{};
            unsigned char green{};
            unsigned char blue{};
            unsigned char alpha{ 255 };
        };

        enum class Effect {
            None = 0,
            Bold = 1,
            Dim = 2,
            Italic = 3,
            Underline = 4,
            Blink = 5,
            Inverse = 7,
            Strikethrough = 9
        };
    public:
        Color fgColor{ 255, 255, 255 };
        Color bgColor{ 0, 0, 0 };
        Effect effect{ Effect::None };
    };
}