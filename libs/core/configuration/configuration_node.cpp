module core.configuration;

using core::configuration::ConfigurationNode;
using core::configuration::ObjectNode;
using core::configuration::ListNode;
using core::configuration::LiteralNode;

ConfigurationNode::ConfigurationNode(ObjectNode&& val)
    : node (std::move(val))
{}

ConfigurationNode::ConfigurationNode(ListNode&& val)
    : node (std::move(val))
{}

ConfigurationNode::ConfigurationNode(LiteralNode&& val)
    : node (std::move(val))
{}

ConfigurationNode::ConfigurationNode(const ObjectNode& val)
    : node (val)
{}

ConfigurationNode::ConfigurationNode(const ListNode& val)
    : node (val)
{}

ConfigurationNode::ConfigurationNode(const LiteralNode& val)
    : node (val)
{}

// Get actual 
const ObjectNode& ConfigurationNode::asObject() const   { return std::get<ObjectNode>(node);  }
const ListNode& ConfigurationNode::asList() const       { return std::get<ListNode>(node);    }
const LiteralNode& ConfigurationNode::asLiteral() const { return std::get<LiteralNode>(node); }

// Given the key, returns the configuration node
const ConfigurationNode& ConfigurationNode::operator[](const std::string& key) const
{
    return asObject()[key];
}

// Given the index, returns the configuration node
const ConfigurationNode& ConfigurationNode::operator[](std::size_t index) const
{
    return asList()[index];
}

std::string ConfigurationNode::toString() const
{
    return std::visit([](auto&& arg){ return arg.toString(); }, node);
}

bool ConfigurationNode::isObject()  const { return std::holds_alternative<ObjectNode>(node);  }
bool ConfigurationNode::isList()    const { return std::holds_alternative<ListNode>(node);    }
bool ConfigurationNode::isLiteral() const { return std::holds_alternative<LiteralNode>(node); }

std::ostream& core::configuration::operator <<(std::ostream& strm, const ConfigurationNode& configurationNode)
{
    return strm << configurationNode.toString();
}
