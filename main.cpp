import std;

import core.version;

int main()
{
    using namespace std;
    cout << "Candela-3 Version: " << candela::version::GetCommitSummary() << " Date: " << candela::version::Date << endl;
    return 0;
}
