module;

#include "cxxopts.hpp"

export module external.cxxopts;

export namespace cxxopts
{
    using cxxopts::Options;
    using cxxopts::value;

    using cxxopts::ParseResult;

    namespace exceptions
    {
        using cxxopts::exceptions::exception;
    }
}
