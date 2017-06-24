#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <string>

using namespace std;

unsigned long long urls_count = 0;
map<string, unsigned long long> domains;
map<string, unsigned long long> paths;

void inc_or_add(map<string, unsigned long long>& m, const string& key)
{
    try
    {
        m.at(key)++;
    }
    catch(const out_of_range& oor)
    {
        m[key] = 0;
    }
}

int main(int argc, char* argv[])
{
    if (argc < 2)
        return 1;
    string ifname = argv[1];
    ifstream infile(ifname);

    string line;
    regex re("https?://([a-zA-Z0-9.-]+)(/[a-zA-Z0-9.,/+_]*)?");
    while(getline(infile, line))
    {
        for (auto i = sregex_iterator(line.begin(), line.end(), re); i != sregex_iterator(); i++)
        {
            urls_count++;
            smatch match = *i;
            inc_or_add(domains, match[1]);
            if (match[2].length() > 0)
                inc_or_add(paths, match[2]);
            else
                inc_or_add(paths, "/");

            // string domain = match[1];
            // string path = match[2];
            // if (path.length() == 0)
            // {
            //     cout << "empty path" << endl;
            //     path = "/";
            // }
            // cout << "domain: " << domain << endl;
            // cout << "path: " << path << endl;
        }
    }

    cout << "total urls " << urls_count << ", domains " << domains.size() << ", paths " << paths.size();

    return 0;
}
