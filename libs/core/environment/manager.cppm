export module core.environment;

import std;
import core.configuration;
import core.factory;

export namespace core::environment
{
    // manager.h
    template <typename T>
    class PairExtractorIterator
    {
    public:
        PairExtractorIterator(T iter)
            : iter(iter)
        {
        }

        auto& operator*() { return (*iter).second; }
        auto operator++() { return ++iter, *this; }
        auto operator!=(const PairExtractorIterator<T>& other) const { return iter != other.iter; }

    private:
        T iter;
    };

    template<typename T>
    class Manager
    {
    public:

        using ItemType = T;
        using ItemPt = typename std::unique_ptr<ItemType>;
        using MapType = typename std::map<std::string, ItemPt>;
        using MapIteratorType = typename MapType::iterator;
        using ListType = typename std::vector<ItemPt>;
        using ListIteratorType = typename ListType::iterator;

        virtual ~Manager() = default;

        template<class U, class ...V>
        void registerItem(const std::string& name, V && ... args)
        {
            itemMap[name] = std::make_unique<U>(std::forward<V>(args)...);
        }

        ItemType& get(const std::string& name)
        {
            auto iterator = itemMap.find(name);
            if (iterator == itemMap.end())
                throw std::runtime_error("Manager - Cannot find instance/factory with name: " + name);

            return *iterator->second;
        }

        const ItemType& get(const std::string& name) const
        {
            auto iterator = itemMap.find(name);
            if (iterator == itemMap.end())
                throw std::runtime_error("Manager - Cannot find instance/factory with name: " + name);

            return *iterator->second;
        }

        bool exists(const std::string& name)
        {
            return itemMap.find(name) != itemMap.end();
        }

        auto begin()
        {
            return PairExtractorIterator<MapIteratorType>(itemMap.begin());
        }

        auto end()
        {
            return PairExtractorIterator<MapIteratorType>(itemMap.end());
        }

        void registerItem(const std::string& name, ItemPt item)
        {
            itemMap[name] = std::move(item);
        }

        void removeItem(const std::string& name)
        {
            itemMap.erase(name);
        }

        std::size_t registerItem(ItemPt item)
        {
            list.push_back(std::move(item));
            return list.size() - 1;
        }

        ItemType& get(std::size_t id) 
        {
            return *list.at(id);
        }

        std::vector<ItemType*> asList()
        {
            std::vector<ItemType*> instances;
            for (auto& i : itemMap)
                instances.push_back(i.second.get());
            for (auto& i : list)
                instances.push_back(i.get());
            return instances;
        }

        ItemType& get(const configuration::ConfigurationNode& config) 
        {
            auto& literal = config.asLiteral();
            return literal.is<std::string>() ? 
                get(literal.read<std::string>()) : 
                get(literal.read<std::uint64_t>());
        }

        void clear()
        {
            list.clear();
            itemMap.clear();
        }

    private:
        MapType itemMap;
        ListType list;
    };

    // Resource_manager.h
    template <typename T, typename F = factory::Factory<T>>
    class ResourceManager
    {
    public:
        using FactoryManagerType = Manager<F>;
        using FactoryType = typename FactoryManagerType::ItemType;
        using FactoryPt = typename FactoryManagerType::ItemPt;
        using InstanceType = T;
        using InstancePt = typename Manager<T>::ItemPt;

        FactoryManagerType& getFactoryManager() { return factoryManager; }
        Manager<T>& getInstanceManager() { return instanceManager; }

        InstancePt createInstance(const std::string& factoryName, const configuration::ConfigurationNode* config = nullptr)
        {
            if (!factoryManager.exists(factoryName))
                throw std::runtime_error(factoryName + " type does not exist");
            auto& factory = factoryManager.get(factoryName);
            return config ? factory.create(*config) : factory.create();
        }

        InstanceType* createInstanceAndSet(const std::string& factoryName, const std::string& instanceName, const configuration::ConfigurationNode* config = nullptr)
        {
            auto instance = createInstance(factoryName, config);
            InstanceType* retPt = instance.get();
            if (instanceName.empty())
                instanceManager.registerItem(std::move(instance));
            else 
                instanceManager.registerItem(instanceName, std::move(instance));
            return retPt;
        }

        // Load configuration
        void loadSection(const std::string& sectionName, configuration::ConfigurationNodePt& configuration, bool optional = false)
        {
            try 
            {
                if (!configuration->asObject().keyExists(sectionName))
                {
                    if (!optional)
                        throw std::runtime_error(sectionName + " section not found");
                    return;
                }

                auto& section = (*configuration)[sectionName];
                if (!section.isList())
                    throw std::runtime_error(sectionName + " is not a list");

                // This line is used for cloning purposes
                setConfigNode(section);

                for (const auto& config : section.asList()) 
                {
                    if (!config.isObject())
                        throw std::runtime_error("Item inside " + sectionName + " is not an object");

                    const auto& confObj = config.asObject();

                    if (!confObj.keyExists("Type"))
                        throw std::runtime_error("Required 'Type' property missing in " + sectionName);
                        
                    auto factoryName = confObj["Type"].read<std::string>();
                    auto instanceName = confObj.keyExists("Name") ? confObj["Name"].read<std::string>() : std::string();
                    createInstanceAndSet(factoryName, instanceName, &config);
                }
            } catch(const std::exception& err) 
            {
                throw std::runtime_error("Error loading config section: " + sectionName + " - " + err.what());
            }
        }

        // These 2 methods allow us to make several copies
        void setConfigNode(const configuration::ConfigurationNode& p_configNode) { configNode = &p_configNode; }

        InstancePt produceFromName(const std::string& name) const
        {
            for (const auto& config : configNode->asList())
            {
                const auto& item = config.asObject();
                if (item.keyExists("Name") && item["Name"].read<std::string>() == name)
                    return factoryManager.get(item["Type"].read<std::string>()).create(config);
            }

            return InstancePt();
        }

        const configuration::ObjectNode& getConfigurationNode(const std::string& name)
        {
            for (const auto& config : configNode->asList())
            {
                const auto& item = config.asObject();
                if (item.keyExists("Name") && item["Name"].read<std::string>() == name)
                    return item;
            }

            throw std::runtime_error("ResourceManager::getConfigurationNode() - Node '" + name + "' not found");
        }

    private:
        const configuration::ConfigurationNode * configNode{};

        FactoryManagerType factoryManager;
        Manager<T> instanceManager;
    };

    // // environment.h
    // class Environment
    // {
    // public:
    //     virtual ~Environment() = default;

    //     static Environment& getInstance();

    // private:
    //     Environment();

    //     void registerFactories();

    //     // Initialise the environment
    //     void init();
    // };
}
