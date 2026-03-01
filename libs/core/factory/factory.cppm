export module core.factory;

import std;
import core.configuration;

export namespace core::factory
{
    // May be subclassed once to be used as a factory for a category of products
    // Or may be subclassed more than once for each product
    template<typename T>
    class Factory
    {
    public:
        virtual ~Factory() = default;
        virtual std::unique_ptr<T> create() const = 0;
        virtual std::unique_ptr<T> create(const configuration::ConfigurationNode& config) const = 0;
    };
}