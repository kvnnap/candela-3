export module core.plugin;

export namespace cpre::plugin
{
    struct PluginMetadata
    {
        const char* name;
        const char* description;
        const char* version;
    };

    class IPlugin
    {
    protected:
        IPlugin() = default;
        virtual ~IPlugin() = default;

    public:
        IPlugin(const IPlugin&) = delete;
        IPlugin(IPlugin&&) = delete;
        IPlugin& operator=(const IPlugin&) = delete;

        virtual PluginMetadata getPluginMetadata() const = 0;
        // virtual void load() = 0;

        // Need to provide requirements for plugin to
        // perhaps in load method, plugin requests dependencies
    };
}
