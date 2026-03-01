// module;

// // Platform-specific export macros
// #if defined(_WIN32) || defined(_WIN64)
//     #define LIB_EXPORT __declspec(dllexport)
//     #define LIB_IMPORT __declspec(dllimport)
// #else
//     #define LIB_EXPORT __attribute__((visibility("default")))
//     #define LIB_IMPORT
// #endif

export module core.system;

import std;

export namespace core::system
{
    using Handle = void*;

    namespace shared_library
    {
        Handle loadLibrary(const std::string& path);
        Handle getSymbol(Handle libraryHandle, const std::string& symbolName);
        bool freeLibrary(Handle library);
        std::vector<std::string> getSymbols(Handle libraryHandle);

        // Without loading functions
        std::vector<std::string> getSymbolsWithoutLoading(const std::string& path);
        bool symbolExistsWithoutLoading(const std::string& path, const std::string& name);
    }

    class SharedLibrary
    {
    public:
        virtual ~SharedLibrary();
        SharedLibrary(const std::string& path);

        bool libraryExists() const;
        bool symbolExists(const std::string& name) const;

        Handle getSymbolRaw(const std::string& name) const;
        std::vector<std::string> getSymbols();

        template<typename T>
        T getSymbol(const std::string& name) const
        {
            auto symbol = getSymbolRaw(name);
            return reinterpret_cast<T>(symbol);
        }

    private:
        Handle libraryHandle;
    };
}