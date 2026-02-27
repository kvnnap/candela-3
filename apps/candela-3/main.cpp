import std;

import core.version;

import core.configuration;

int main()
{
    using namespace std;
    cout << "Candela-3 Version: " << candela::version::GetCommitSummary() << " Date: " << candela::version::Date << endl;

    using namespace core::configuration;
    ConfigurationNode a = LiteralNode(2);
    
    cout << "a: " << a << endl;

    // We need to read the configuration and launch a renderer
    
    return 0;
}
