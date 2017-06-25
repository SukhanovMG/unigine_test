#include <algorithm>
#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <vector>

using namespace std;

typedef map<string, unsigned long long> string_counter;

unsigned long long urls_count = 0;
string_counter domains;
string_counter paths;

// инкрементировать счётчик ключа или добавить новый
void inc_or_add(string_counter& m, const string& key)
{
    try
    {
        m.at(key)++;
    }
    catch(const out_of_range& oor)
    {
        m[key] = 1;
    }
}

// показать топ ключей в string_counter, по количеству значений
void show_map_top(ostream& os, string_counter& m, size_t count)
{
    vector<pair<string, unsigned long long>> top_m(count > 0 ? count : m.size());
    auto top_end = partial_sort_copy(m.begin(), m.end(), top_m.begin(), top_m.end(),
                      [](pair<string, unsigned long long> l, pair<string, unsigned long long> r)
                      {
                        return l.second != r.second ? l.second > r.second : l.first < r.first;
                      });
    top_m.erase(top_end, top_m.end());
    for (auto& d : top_m)
    {
        os << d.second << " " << d.first << endl;
    }
}

// показать статистику, топ доменов и путей
void show_top(ostream& os, string_counter& domains, string_counter& paths, size_t count)
{
    os << "total urls " << urls_count << ", domains " << domains.size() << ", paths " << paths.size() << endl;
    os << endl;
    os << "top domains" << endl;
    show_map_top(os, domains, count);
    os << endl;
    os << "top paths" << endl;
    show_map_top(os, paths, count);
}

// прочитать аргументы командной строки
void read_args(int argc, char* argv[], size_t& n, string& ifname, string& ofname)
{
    size_t argn = 1;
    if (argc < 2)
        throw runtime_error("Too few arguments");
    if(string(argv[argn]) == "-n")
    {
        if (argc < 5)
            throw runtime_error("Too few arguments");
        argn++;
        size_t _n;
        try
        {
            _n = stoull(argv[argn]);
        }
        catch(const invalid_argument&)
        {
            throw runtime_error("N must be number");
        }
        catch(const out_of_range&)
        {
            throw runtime_error("N is out of bounds");
        }
        argn++;
        n = _n;
    }
    else
    {
        n = 0;
        if (argc < 3)
            throw runtime_error("Too few arguments");
    }
    ifname = argv[argn++];
    ofname = argv[argn];
}

int main(int argc, char* argv[])
{
    try
    {
        string ifname, ofname;
        size_t N;

        read_args(argc, argv, N, ifname, ofname);

        ifstream infile(ifname);
        if (!infile.is_open())
            throw runtime_error("Can't open input file");
        ofstream outfile(ofname);
        if (!outfile.is_open())
            throw runtime_error("Can't open output file");

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
            }
        }

        show_top(outfile, domains, paths, N);
    }
    catch(const runtime_error& re)
    {
        cerr << re.what() << endl;
    }
    catch(const bad_alloc& e)
    {
        cerr << "Not enough memory" << endl;
    }
    catch(const exception& e)
    {
        cerr << "Internal error: " << e.what() << endl;
    }

    return 0;
}
