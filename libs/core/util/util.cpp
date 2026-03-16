module core.util;

std::vector<std::byte> core::util::loadBinaryFile(const std::string& path)
{
    std::vector<std::byte> buffer(std::filesystem::file_size(path));
    std::ifstream file(path, std::ios::binary);
    if (!file)
        throw std::runtime_error("failed to open file: " + path);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), buffer.size()))
        throw std::runtime_error("failed to read file: " + path);
    return buffer;
}
