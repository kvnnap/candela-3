export module core.configuration:list;

import std;

export namespace core::configuration
{
    class ConfigurationNode;

    class ListNode
    {
    public:
        using ListNodeCollection = std::vector<ConfigurationNode>;
        using ListNodeCollectionIterator = ListNodeCollection::const_iterator;

        ListNode() = default;
        ListNode(ListNode&& other) = default;
        ListNode(const ListNode& other) = default;
        ListNode& operator=(ListNode&& rhs) = default;
        ListNode& operator=(const ListNode& rhs) = default;

        const ConfigurationNode& operator[](std::size_t index) const;
        std::size_t size() const;

        void add(ConfigurationNode&& configurationNode);

        std::string toString() const;

        ListNodeCollectionIterator begin() const;
        ListNodeCollectionIterator end() const;
    private:
        ListNodeCollection nodeVector;
    };
}