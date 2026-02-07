module core.version;

import std;

#if __has_include("auto_version.h")
#include "auto_version.h"
#else
#define CANDELA_DIRTY
#endif

#ifndef CANDELA_COMMIT
#define CANDELA_COMMIT N/A
#endif

#ifndef CANDELA_DATE
#define CANDELA_DATE N/A
#endif

#ifdef CANDELA_DIRTY
#undef CANDELA_DIRTY
#define CANDELA_DIRTY true
#else
#define CANDELA_DIRTY false
#endif

#define STRINGIFY2(X) #X
#define STRINGIFY(X) STRINGIFY2(X)

const char* candela::version::Commit = STRINGIFY(CANDELA_COMMIT);
const char* candela::version::Date = STRINGIFY(CANDELA_DATE);
const bool candela::version::Dirty = CANDELA_DIRTY;

static char commitSummary[16] = {};
static const char dirty[] = "-dirty";

const char* candela::version::GetCommitSummary()
{
    using namespace std;   
	if (commitSummary[0] == '\0')
	{
        constexpr auto hashLength = 7u;
		memcpy(&commitSummary[0], &Commit[0], hashLength);
		if (Dirty)
		    memcpy(&commitSummary[hashLength], &dirty[0], sizeof(dirty));
	}
	return commitSummary;
}