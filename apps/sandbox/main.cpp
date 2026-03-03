import sandbox.std;
// import vulkan;

int main()
{
    std::string formatted_str = std::format("My name is {} and my favorite number is {}", "Kevin", 127);
    std::cout << formatted_str << std::endl;
    return 0;
}