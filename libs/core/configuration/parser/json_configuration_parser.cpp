module;

#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"

module core.configuration.parser;

using std::string;
using std::vector;
using std::make_unique;
using std::invalid_argument;
using std::runtime_error;

using rapidjson::Value;
using rapidjson::Document;
using rapidjson::FileReadStream;
using rapidjson::StringStream;
using rapidjson::kFalseType;
using rapidjson::kTrueType;
using rapidjson::kStringType;
using rapidjson::kNumberType;
using rapidjson::kNumberType;
using rapidjson::kNullType;

using core::configuration::ObjectNode;
using core::configuration::LiteralNode;
using core::configuration::ListNode;
using core::configuration::ConfigurationNode;
using core::configuration::ConfigurationNodePt;
using core::configuration::parser::ParserPt;
using core::configuration::parser::JsonConfigurationParser;
using core::configuration::parser::JsonConfigurationParserFactory;

// C-style static - hide this symbol
static ConfigurationNode fromValue(Value &value)
{
    if (value.IsObject())
    {
        ObjectNode objectNode;
        for(auto& v : value.GetObject())
            objectNode.add(v.name.GetString(), make_unique<ConfigurationNode>(fromValue(v.value)));
        return objectNode;
    } else if (value.IsArray())
    {
        ListNode listNode;
        for(auto& v : value.GetArray())
            listNode.add(fromValue(v));
        return listNode;
    } else
    {
        switch (value.GetType()) {
            case kFalseType: return LiteralNode(false);
            case kTrueType:  return LiteralNode(true);
            case kStringType: return LiteralNode(string(value.GetString()));
            case kNumberType:
                if (value.IsUint64())
                    return LiteralNode(value.GetUint64());
                else if (value.IsInt64())
                    return LiteralNode(value.GetInt64());
                else
                    return LiteralNode(value.GetDouble());
            case kNullType:
            default:
                return LiteralNode(string("null"));
        }
    }
}

void JsonConfigurationParser::setFileName(const string& p_fileName)
{
    fileNameOrJsonText = p_fileName;
    isFileName = true;
}

void JsonConfigurationParser::setJsonText(const std::string& jsonText)
{
    fileNameOrJsonText = jsonText;
    isFileName = false;
}

std::string JsonConfigurationParser::getType() const
{
    return "json";
}

JsonConfigurationParser::JsonConfigurationParser()
    : isFileName()
{
}

ConfigurationNodePt JsonConfigurationParser::loadConfiguration() const
{
    // Validate file name
    if (fileNameOrJsonText.empty())
        throw invalid_argument("JsonConfigurationParser: fileName or jsonText cannot be empty");

    Document document;
    if (isFileName)
    {
        // Open config file
        FILE* fp = fopen(fileNameOrJsonText.c_str(), "r");
        if (fp == nullptr) {
            throw runtime_error("JsonConfigurationParser: supplied file name does not exist");
        }

        // Initialise buffer on heap
        {
            vector<char> buffer (65536);
            FileReadStream fileReadStream (fp, buffer.data(), buffer.size());
            document.ParseStream(fileReadStream);
        }

        // close file and clear buffer
        fclose(fp);
    } else
    {
        StringStream stringStream(fileNameOrJsonText.c_str());
        document.ParseStream(stringStream);
    }

    // validate root node
    if (!document.IsObject() && !document.IsArray())
        throw runtime_error("JsonConfigurationParser: Invalid root - not object nor array");

    // Valid root, start parsing
    return make_unique<ConfigurationNode>(fromValue(document));
}

// Factory
ParserPt JsonConfigurationParserFactory::create() const
{
    return make_unique<JsonConfigurationParser>();
}

ParserPt JsonConfigurationParserFactory::create(const ConfigurationNode &config) const
{
    auto parser = make_unique<JsonConfigurationParser>();
    const auto &confObj = config.asObject();
    if (confObj.keyExists("file"))
        parser->setFileName(confObj["file"].read<std::string>());
    else if (confObj.keyExists("json"))
        parser->setJsonText(confObj["json"].read<std::string>());
    return parser;
}
