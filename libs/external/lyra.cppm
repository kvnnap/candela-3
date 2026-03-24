module;

#include "lyra/lyra.hpp"

export module external.lyra;

export namespace lyra
{
    using lyra::parser_result; // msvc needs this
    using lyra::cli;
    using lyra::opt;
    using lyra::arg;
    using lyra::help;
}
