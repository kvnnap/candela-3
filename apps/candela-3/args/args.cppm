export module candela.args;

import std;

export namespace candela::args
{
    class CandelaArguments
    {
        
        public:

            CandelaArguments(const std::string& p_description);

            void parse(int argc, const char * const * argv);

            bool error;
            bool help;

            std::string description;
            
            std::string configFile;
            std::string errorMessage;
    };
}