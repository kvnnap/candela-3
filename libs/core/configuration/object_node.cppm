export module core.configuration:object;

import std;

import :object_iterator;

export namespace core::configuration
{
    class ObjectNode
    {
    public:
        ObjectNode() = default;
        ObjectNode(ObjectNode&& other) noexcept = default;
        ObjectNode(const ObjectNode& other);
        ObjectNode& operator=(ObjectNode&& rhs) noexcept = default;
        ObjectNode& operator=(const ObjectNode& rhs);

        const ConfigurationNode& operator[](const std::string& key) const;
        bool keyExists(const std::string& key) const;
        std::size_t size() const;

        void add(const std::string& key, std::unique_ptr<ConfigurationNode> configurationNode);

        std::string toString() const;

        ObjectNodeIterator begin() const;
        ObjectNodeIterator end() const;

    private:
        object_node_types::ObjectNodeCollection nodeMap;
    };

}