import std;

import core.version;
import candela.args;
import candela.renderer;

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::exception;

using candela::args::CandelaArguments;

using candela::version::GetCommitSummary;
using candela::version::Date;

using candela::renderer::VulkanRenderer;

int main(int argc, const char * const * argv)
{
    CandelaArguments ca { 
        "Candela 3 - A physically based renderer. Version: " + string(GetCommitSummary()) +
        ", Date: " + string(Date) };
    ca.configFile = "config.json";
    ca.parse(argc, argv);

    if (ca.error || ca.help)
        return ca.error ? -1 : 0;
    

    // cout << "Loading: " << ca.configFile << endl;

    // We need to read the configuration and launch a renderer

    try 
    {
        VulkanRenderer vk;
        vk.initWindow();
        vk.init();

        while(true)
        {
            // run window message loop
            if (vk.processMessages())
                break;
            vk.renderFrame();
        }
        
        vk.cleanup();

    } catch (const exception& e) 
    {
        cerr << "Exception: " << e.what() << endl;
        return -1;
    }

    return 0;
}