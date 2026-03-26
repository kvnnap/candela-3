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
        // Needed if want to enable copy constructor, as it gets deleted in g++
        // ObjectNodeIterator(const ObjectNodeIterator&) = default;
        
        ObjectNodeIterator(object_node_types::ObjectNodeCollectionIterator iter);

        ObjectItem operator*();
        ObjectNodeIterator& operator++(); 
        bool operator!=(const ObjectNodeIterator& other) const;

    private:
        object_node_types::ObjectNodeCollectionIterator iter;
    };
}
