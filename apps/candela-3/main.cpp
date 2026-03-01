import std;

import core.version;
import candela.args;

using std::cout;
using std::endl;
using std::string;

using candela::args::CandelaArguments;

using candela::version::GetCommitSummary;
using candela::version::Date;

int main(int argc, const char * const * argv)
{
    CandelaArguments ca { 
        "Candela 3 - A physically based renderer. Version: " + string(GetCommitSummary()) +
        ", Date: " + string(Date) };
    ca.configFile = "config.json";
    ca.parse(argc, argv);

    if (ca.error || ca.help)
        return ca.error ? -1 : 0;
    

    cout << "Loading: " << ca.configFile << endl;

    // We need to read the configuration and launch a renderer

    return 0;
}