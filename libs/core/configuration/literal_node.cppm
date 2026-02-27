export module core.configuration:literal;

import std;

export namespace core::configuration
{
    class LiteralNode
    {
    public:
        LiteralNode(LiteralNode&& other) = default;
        LiteralNode(const LiteralNode& other) = default;
        LiteralNode& operator=(LiteralNode&& rhs) = default;
        LiteralNode& operator=(const LiteralNode& rhs) = default;

        template <class T>
        LiteralNode(T&& val)
        {
            value = std::move(val);
        }

        template <typename T>
        T read() const
        {
            if constexpr(std::is_same_v<T, float>)
                return static_cast<T>(read<double>());
            else if constexpr(std::is_same_v<T, std::uint32_t>)
                return static_cast<T>(read<std::uint64_t>());
            else if constexpr(std::is_same_v<T, std::int32_t>)
                return static_cast<T>(read<std::int64_t>());
            else {
                //return std::get<T>(value);
                if (is<T>())
                    return std::get<T>(value);

                // apply fallback
                if constexpr(std::is_same_v<T, std::int64_t>)
                {
                    if (is<std::uint64_t>()) 
                    {
                        T t = static_cast<T>(std::get<std::uint64_t>(value));
                        if (t >= 0) 
                            return t;
                    }
                } else if constexpr(std::is_same_v<T, double>)
                {
                    if (is<std::uint64_t>()) 
                        return static_cast<T>(std::get<std::uint64_t>(value));
                    else if (is<std::int64_t>()) 
                        return static_cast<T>(std::get<std::int64_t>(value));
                }

                return std::get<T>(value);
            }
        }

        template <typename T>
        bool is() const
        {
            return std::holds_alternative<T>(value);
        }

        std::string toString() const;
    private:
        std::variant<std::uint64_t, std::int64_t, double, bool, std::string> value;
    };
}
