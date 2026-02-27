module core.configuration;

using core::configuration::ObjectNodeIterator;
using core::configuration::ObjectItem;

ObjectNodeIterator::ObjectNodeIterator(object_node_types::ObjectNodeCollectionIterator iter)
    : iter (iter)
{
}

ObjectItem ObjectNodeIterator::operator*()
{
    const auto& x = *iter;
    return ObjectItem{ x.first, *x.second };
}

ObjectNodeIterator ObjectNodeIterator::operator++()
{
    return ++iter, *this;
}

bool ObjectNodeIterator::operator!=(const ObjectNodeIterator& other) const
{
    return iter != other.iter;
}
