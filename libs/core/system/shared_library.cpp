module;

#if defined(__unix__)

#include <dlfcn.h>
#include <link.h>
#include <link.h>     // For ElfW, Elf64_Sym, etc.
#include <elf.h>      // For ELF symbol types

#elif defined(_WIN32) || defined(_WIN64)

#include <windows.h>
#include <imagehlp.h>

#endif

module core.system;

import core.util;

using namespace core::system::shared_library;
using core::system::Handle;
using std::string;
using std::vector;

#if defined(__unix__)

Handle core::system::shared_library::loadLibrary(const string& path)
{
    return dlopen(path.c_str(), RTLD_LAZY);
}

Handle core::system::shared_library::getSymbol(Handle libraryHandle, const string& symbolName)
{
    return dlsym(libraryHandle, symbolName.c_str());
}

bool core::system::shared_library::freeLibrary(Handle library)
{
    return dlclose(library) == 0;
}

vector<string> core::system::shared_library::getSymbols(Handle handle)
{
    vector<string> symbols;
    struct link_map *lm;
    dlinfo(handle, RTLD_DI_LINKMAP, &lm);

    // Find the dynamic section
    ElfW(Dyn) *dynamic = lm->l_ld;
    ElfW(Sym) *symtab = nullptr;
    char *strtab = nullptr;
    uint32_t *hash = nullptr;
    size_t symentries = 0;
    
    // Iterate over the dynamic section to find needed tables
    for (ElfW(Dyn) *entry = dynamic; entry->d_tag != DT_NULL; entry++) 
    {
        switch (entry->d_tag) 
        {
            case DT_SYMTAB:
                symtab = (ElfW(Sym) *)entry->d_un.d_ptr;
                break;
            case DT_STRTAB:
                strtab = (char *)entry->d_un.d_ptr;
                break;
            case DT_HASH:
                hash = (uint32_t *)entry->d_un.d_ptr;
                break;
            case DT_GNU_HASH:
                // For GNU hash format, we'd need to parse it differently
                // This implementation uses only DT_HASH for simplicity
                break;
        }
    }
    
    if (!symtab || !strtab || !hash) {
        return symbols;  // return empty if essentials are missing
    }
    
    // The second entry in the hash table is the number of symbols
    symentries = hash[1];
    symbols.reserve(symentries);

    // Walk through the symbol table
    for (size_t i = 0; i < symentries; i++) {
        ElfW(Sym) *sym = &symtab[i];
        if (sym->st_name != 0) {  // Skip entries with no name
            const char *name = strtab + sym->st_name;
            if (ELF64_ST_TYPE(sym->st_info) == STT_FUNC || ELF64_ST_TYPE(sym->st_info) == STT_OBJECT) {
                symbols.emplace_back(name);
            }
        }
    }
    
    return symbols;
}

vector<string> core::system::shared_library::getSymbolsWithoutLoading(const string& path)
{
    return vector<string>();
}

#elif defined(_WIN32) || defined(_WIN64)

using core::util::ScopeGuard;

Handle core::system::shared_library::loadLibrary(const string& path)
{
    return LoadLibrary(path.c_str());
}

Handle core::system::shared_library::getSymbol(Handle libraryHandle, const string& symbolName)
{
    return reinterpret_cast<Handle>(GetProcAddress(static_cast<HMODULE>(libraryHandle), symbolName.c_str()));
}

bool core::system::shared_library::freeLibrary(Handle library)
{
    return FreeLibrary(static_cast<HMODULE>(library));
}

vector<string> core::system::shared_library::getSymbols(Handle hModule)
{
    vector<string> symbols;

    // Get the base address of the DLL
    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)hModule;
    PIMAGE_NT_HEADERS ntHeader = (PIMAGE_NT_HEADERS)((BYTE*)hModule + dosHeader->e_lfanew);
    PIMAGE_EXPORT_DIRECTORY exportDir = (PIMAGE_EXPORT_DIRECTORY)((BYTE*)hModule + ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

    symbols.reserve(exportDir->NumberOfNames);
    DWORD *names = (DWORD*)((BYTE*)hModule + exportDir->AddressOfNames);
    for (DWORD i = 0; i < exportDir->NumberOfNames; ++i)
        symbols.emplace_back((char*)((BYTE*)hModule + names[i]));
    
    return symbols;
}

// #pragma comment(lib, "imagehlp.lib") or in CMAKE
vector<string> core::system::shared_library::getSymbolsWithoutLoading(const string& dllPath)
{
    vector<string> symbols;

    LOADED_IMAGE loadedImage{};
    BOOL res {};
    auto sg = ScopeGuard([&]() { res = MapAndLoad(dllPath.c_str(), nullptr, &loadedImage, TRUE, TRUE); },
                         [&]() { if (res) UnMapAndLoad(&loadedImage); });
    if (!res)
        return symbols;

    // Get export directory
    ULONG size;
    auto exportDir = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(ImageDirectoryEntryToDataEx(
        loadedImage.MappedAddress, FALSE, IMAGE_DIRECTORY_ENTRY_EXPORT, &size, NULL));
    if (!exportDir)
        return symbols;

    auto names = reinterpret_cast<DWORD*>(ImageRvaToVa(loadedImage.FileHeader, loadedImage.MappedAddress, exportDir->AddressOfNames, nullptr));

    symbols.reserve(exportDir->NumberOfNames);
    for (DWORD i = 0; i < exportDir->NumberOfNames; i++)
        symbols.emplace_back(reinterpret_cast<const char*>(ImageRvaToVa(loadedImage.FileHeader, loadedImage.MappedAddress, names[i], nullptr)));

    return symbols;
}

#ifdef SL_OPTIONAL
// Helper function to convert RVA to file offset
DWORD RvaToFileOffset(IMAGE_NT_HEADERS* ntHeader, DWORD rva)
{
    IMAGE_SECTION_HEADER* section = IMAGE_FIRST_SECTION(ntHeader);
    for (WORD i = 0; i < ntHeader->FileHeader.NumberOfSections; ++i)
    {
        if (rva >= section->VirtualAddress && rva < section->VirtualAddress + section->Misc.VirtualSize)
            return (rva - section->VirtualAddress) + section->PointerToRawData;
        ++section;
    }
    return rva; // Fallback if not found in any section
}

vector<string> getSymbolsWithoutLoading2(const string& dllPath)
{
    vector<string> exports;
    char path[MAX_PATH];
    if (!SearchPath(NULL, dllPath.c_str(), ".dll", sizeof(path), path, nullptr))
        return exports;
    
    // Open the DLL file
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open())
        return exports;
    
    // Get file size and read entire contents
    auto size = file.tellg();
    file.seekg(0, std::ios::beg);

    vector<char> buffer(size);
    if (!file.read(buffer.data(), size))
        return exports;

    // Parse PE headers
    auto dosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(buffer.data());
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
        return exports;

    auto ntHeader = reinterpret_cast<IMAGE_NT_HEADERS*>(buffer.data() + dosHeader->e_lfanew);
    if (ntHeader->Signature != IMAGE_NT_SIGNATURE)
        return exports;

    // Check for exports section
    auto &dirExport = ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    if (dirExport.Size == 0)
        return exports;

    // Get export directory (relative to file offset)
    auto exportDir = reinterpret_cast<IMAGE_EXPORT_DIRECTORY*>(buffer.data() + RvaToFileOffset(ntHeader, dirExport.VirtualAddress));
    auto names = reinterpret_cast<DWORD*>(buffer.data() + RvaToFileOffset(ntHeader, exportDir->AddressOfNames));

    // Collect all export names
    exports.reserve(exportDir->NumberOfNames);
    for (DWORD i = 0; i < exportDir->NumberOfNames; ++i)
        exports.emplace_back(reinterpret_cast<const char*>(buffer.data() + RvaToFileOffset(ntHeader, names[i])));

    return exports;
}
#endif

bool core::system::shared_library::symbolExistsWithoutLoading(const string& path, const string& name)
{
    auto symbols = getSymbolsWithoutLoading(path);
    for (const auto& s : symbols)
        if (s == name)
            return true;
    return false;
}

#endif

// Class stuff
using core::system::SharedLibrary;

SharedLibrary::~SharedLibrary()
{
    if (libraryHandle)
        freeLibrary(libraryHandle);
    libraryHandle = nullptr;
}

SharedLibrary::SharedLibrary(const string& path)
    : libraryHandle(nullptr)
{
    libraryHandle = loadLibrary(path);
}

bool SharedLibrary::libraryExists() const
{
    return libraryHandle != nullptr;
}

bool SharedLibrary::symbolExists(const string& name) const
{
    return getSymbolRaw(name) != nullptr;
}

Handle SharedLibrary::getSymbolRaw(const string& name) const
{
    return shared_library::getSymbol(libraryHandle, name);
}

vector<string> SharedLibrary::getSymbols()
{
    return shared_library::getSymbols(libraryHandle);
}
