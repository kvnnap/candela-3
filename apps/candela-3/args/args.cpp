module;

#include "lyra/lyra.hpp"

module candela.args;

using candela::args::CandelaArguments;

CandelaArguments::CandelaArguments(const std::string& p_desc)
    : error(), help(), description(p_desc)
{}

void CandelaArguments::parse(int argc, const char * const * argv)
{
    auto cli = lyra::cli()
    | lyra::opt( this->configFile, "Configuration file name" ) ["-c"]["--config"]("Configuration file name")
    | lyra::help(this->help).description(description);
    auto result = cli.parse( { argc, argv } );

    if (this->help)
        std::cout << cli << std::endl;

    if ( (this->error = !result) )
    {
        errorMessage = "Error in command line: " + result.message();
        std::cerr << errorMessage << std::endl;
    }
}