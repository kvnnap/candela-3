module core.configuration;

using core::configuration::LiteralNode;

// helper type for the visitor #4
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
// explicit deduction guide (not needed as of C++20)
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

std::string LiteralNode::toString() const
{
    return std::visit(overloaded {
        [](auto arg) { return std::to_string(arg); },
        [](bool arg) { return std::string(arg ? "true" : "false"); },
        [](const std::string& arg) { return "\"" + arg + "\""; },
    }, value);
}
