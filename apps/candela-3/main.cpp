import std;

import core.version;
import external.cxxopts;

using std::cout;
using std::endl;
using std::string;

using cxxopts::Options;
using cxxopts::value;
using cxxopts::ParseResult;
using cxxopts::exceptions::exception;

using candela::version::GetCommitSummary;
using candela::version::Date;

int main(int argc, const char *argv[])
{
    const auto description =  
        "Candela 3 - A physically based renderer. Version: " + string(GetCommitSummary()) +
        ", Date: " + string(Date);

    Options options("candela-3", description);
    options.add_options()
        ("c,config", "Configuration file name", value<string>()->default_value("config.json"))
        ("h,help", "Print usage");
        

    ParseResult result;
    try {
        result = options.parse(argc, argv);
    } catch (exception &e) {
        cout << e.what() << endl;
        return -1;
    }
    
    if (result.count("help"))
    {
        cout << options.help() << endl;
        return 0;
    }

    cout << description << endl;
    auto configFileName = result["c"].as<std::string>();

    cout << "Loading: " << configFileName << endl;

    // using namespace core::configuration;
    // ConfigurationNode a = LiteralNode(2);
    
    // cout << "a: " << a << endl;

    // We need to read the configuration and launch a renderer
    
    return 0;
}
