module core.configuration;

using std::string;

using core::configuration::ConfigurationNode;
using core::configuration::ObjectNode;
using core::configuration::ObjectNodeIterator;
using core::configuration::object_node_types::ObjectNodeCollection;

const ConfigurationNode& ObjectNode::operator[](const std::string& key) const
{
    auto iterator = nodeMap.find(key);
    if (iterator == nodeMap.end())
        throw std::runtime_error("Key '" + key + "' not found in ObjectNode");

    return *iterator->second;
}

bool ObjectNode::keyExists(const std::string& key) const
{
    return nodeMap.find(key) != nodeMap.end();
}

std::size_t ObjectNode::size() const
{
    return nodeMap.size();
}

void ObjectNode::add(const std::string& key, std::unique_ptr<ConfigurationNode> configurationNode)
{
    nodeMap.insert_or_assign(key, std::move(configurationNode));
}

string ObjectNode::toString() const
{
    string returnString;
    returnString += '{';
    for (const auto& child : *this) {
        returnString += '"';
        returnString += child.key;
        returnString += "\": ";
        returnString += child.value.toString();
        returnString += ", ";
    }

    // Remove last comma
    if (returnString.length() > 2)
        returnString.erase(returnString.length() - 2);

    returnString += '}';
    return returnString;
}

ObjectNodeIterator ObjectNode::begin() const
{
    return nodeMap.begin();
}

ObjectNodeIterator ObjectNode::end() const
{
    return nodeMap.end();
}

ObjectNode::ObjectNode(const ObjectNode& other)
{
    *this = other;
}

ObjectNode& ObjectNode::operator=(const ObjectNode& rhs)
{
    nodeMap.clear();
    for(auto const& x : rhs.nodeMap)
        nodeMap[x.first] = std::make_unique<ConfigurationNode>(*x.second);
    return *this;
}
