template <>
struct std::formatter<exo::var_base, char> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> typename ParseContext::iterator {
        auto it = ctx.begin();
        while (it != ctx.end() && *it != '}') ++it;
        if (it != ctx.end()) ++it;
        return it;
    }

    template <typename FormatContext>
    auto format(const exo::var_base& v, FormatContext& ctx) const -> typename FormatContext::iterator {
        return std::format_to(ctx.out(), "{}", std::string_view(v));
    }
};