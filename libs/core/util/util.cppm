export module core.util;

import std;

export namespace core::util
{

class ScopeGuard {
public:
    ScopeGuard(const std::function<void(void)>& start, const std::function<void(void)>& end)
        : end (end)
    {
        start();
    }

    ~ScopeGuard() { end(); }
    
    ScopeGuard(ScopeGuard const&) = delete;
    void operator=(ScopeGuard const&) = delete;
private:
    const std::function<void(void)> end;
};

}