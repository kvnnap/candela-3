export module core.configuration:node;

import std;

import :literal;
import :list;
import :object;

export namespace core::configuration
{
    class ConfigurationNode
    {
    public:
        ConfigurationNode(ConfigurationNode&& other) = default;
        ConfigurationNode(const ConfigurationNode& other) = default;
        ConfigurationNode& operator=(ConfigurationNode&& rhs) = default;
        ConfigurationNode& operator=(const ConfigurationNode& rhs) = default;

        ConfigurationNode(ObjectNode&& val);
        ConfigurationNode(ListNode&& val);
        ConfigurationNode(LiteralNode&& val);
        
        ConfigurationNode(const ObjectNode& val);
        ConfigurationNode(const ListNode& val);
        ConfigurationNode(const LiteralNode& val);

        // Get actual 
        const ObjectNode& asObject() const;
        const ListNode& asList() const;
        const LiteralNode& asLiteral() const;

        // Given the key, returns the configuration node
        const ConfigurationNode& operator[](const std::string& key) const;

        // Given the index, returns the configuration node
        const ConfigurationNode& operator[](std::size_t index) const;

        // String Representation of the whole object
        std::string toString() const;

        bool isObject() const;
        bool isList() const;
        bool isLiteral() const;

        template <class T>
        T read() const 
        {
            return asLiteral().read<T>();
        }
    private:
        std::variant<ObjectNode, ListNode, LiteralNode> node;
    };

    std::ostream& operator<< (std::ostream& strm, const ConfigurationNode& configurationNode);

    // Types
    using ConfigurationNodePt = std::unique_ptr<ConfigurationNode>;
}