export module core.configuration.parser;

import std;
import core.configuration;
import core.factory;

export namespace core::configuration::parser
{
    class Parser 
    {
    public:
        virtual ~Parser() = default;
        virtual ConfigurationNodePt loadConfiguration() const = 0;
        virtual std::string getType() const = 0;
    };

    using ParserPt = std::unique_ptr<Parser>;

    class JsonConfigurationParser
        : public Parser
    {
    public:
        JsonConfigurationParser();

        ConfigurationNodePt loadConfiguration() const override;
        std::string getType() const override;
        void setFileName(const std::string& fileName);
        void setJsonText(const std::string& jsonText);
    private:
        std::string fileNameOrJsonText;
        bool isFileName;
    };

    class JsonConfigurationParserFactory
        : public factory::Factory<Parser>
    {
    public:
        ParserPt create() const override;
        ParserPt create(const configuration::ConfigurationNode& config) const override;
    };
    
}