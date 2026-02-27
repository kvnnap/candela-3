export module core.configuration:object_types;

import std;

export namespace core::configuration
{
    class ConfigurationNode;

    namespace object_node_types
    {
        using ObjectNodeCollection = std::unordered_map<std::string, std::unique_ptr<ConfigurationNode>>;
        using ObjectNodeCollectionIterator = ObjectNodeCollection::const_iterator;
    }
}