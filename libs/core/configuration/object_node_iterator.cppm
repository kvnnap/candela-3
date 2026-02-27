export module core.configuration:object_iterator;

import std;

import :object_types;

namespace core::configuration
{
    export struct ObjectItem
    {
        const std::string& key;
        const ConfigurationNode& value;
    };

    export class ObjectNodeIterator
    {
    public:
        ObjectNodeIterator(object_node_types::ObjectNodeCollectionIterator iter);

        ObjectItem operator*();
        ObjectNodeIterator operator++(); 
        bool operator!=(const ObjectNodeIterator& other) const;

    private:
        object_node_types::ObjectNodeCollectionIterator iter;
    };
}
