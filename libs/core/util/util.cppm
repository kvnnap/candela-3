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

std::vector<std::byte> loadBinaryFile(const std::string& path);

// TODO: Need to optimise the below if lists are large
template<std::ranges::input_range Set, std::ranges::input_range Elems, typename Comp>
requires std::predicate<Comp, const std::ranges::range_value_t<Set>&, const std::ranges::range_value_t<Elems>&>
bool AllElemsInSet(const Set& set, const Elems& elems, Comp comp)
{
    return std::ranges::all_of(elems, [&](const auto& e) {
        return std::ranges::any_of(set, [&](const auto& s) {
            return comp(s, e);
        });
    });
}

// TODO: Need to optimise the below if lists are large
template<std::ranges::input_range Set, std::ranges::input_range Elems, typename Comp>
requires std::predicate<Comp, const std::ranges::range_value_t<Set>&, const std::ranges::range_value_t<Elems>&>
std::vector<const std::ranges::range_value_t<Elems>*> GetElemsNotInSet(const Set& set, const Elems& elems, Comp comp)
{
    std::vector<const std::ranges::range_value_t<Elems>*> eNotFound;
    for(const auto& e : elems)
    {
        const auto b = std::ranges::any_of(set, [&](const auto& s) {
            return comp(s, e);
        });

        if (!b)
            eNotFound.push_back(&e);
    }

    return eNotFound;
}

// FPS counter for renderers
class FpsCounter
{
public:
	FpsCounter();

	bool hitFrame();
	void resetFrameCount();

	std::uint64_t getFrameCount() const;
	float getFramesPerSecond() const;
	std::uint64_t getLastFrameTime() const;
private:
	std::uint64_t frames, framesPrev, viewFrames;
	float fps;
	std::chrono::milliseconds ms;
	std::chrono::milliseconds prevMs, currentMs;
};

}