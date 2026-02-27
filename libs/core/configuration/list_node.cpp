module core.configuration;

using core::configuration::ConfigurationNode;
using core::configuration::ListNode;
using std::string;

const ConfigurationNode& ListNode::operator[](std::size_t index) const
{
    return nodeVector.at(index);
}

std::size_t ListNode::size() const
{
    return nodeVector.size();
}

void ListNode::add(ConfigurationNode&& configurationNode)
{
    nodeVector.emplace_back(std::move(configurationNode));
}

string ListNode::toString() const
{
    string returnString;
    returnString += '[';
    
    for (const auto& child : *this)
    {
        returnString += child.toString();
        returnString += ", ";
    }

    if (returnString.length() > 2)
        returnString.erase(returnString.length() - 2);
        
    returnString += ']';
    return returnString;
}

ListNode::ListNodeCollectionIterator ListNode::begin() const
{
    return nodeVector.begin();
}

ListNode::ListNodeCollectionIterator ListNode::end() const
{
    return nodeVector.end();
}

