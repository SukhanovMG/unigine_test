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

void process_line(const string& line, regex& re)
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

typedef string::const_iterator scit;
typedef pair<scit, scit> find_res;

enum class pstate {
    h = 0, t, t2, p, s, scol, slash, slash2, end
};

find_res find_proto(scit begin, scit end)
{
    pstate state = pstate::h;
    find_res res = make_pair(end, end);
    for (scit c = begin; c != end; c++)
    {
        switch(state)
        {
        case pstate::h:
            if (*c == 'h')
            {
                state = pstate::t;
                res.first = c;
            }
            break;
        case pstate::t:
            if (*c == 't') state = pstate::t2; else state = pstate::h;
            break;
        case pstate::t2:
            if (*c == 't') state = pstate::p; else state = pstate::h;
            break;
        case pstate::p:
            if (*c == 'p') state = pstate::s; else state = pstate::h;
                break;
        case pstate::s:
            if (*c == 's')
            {
                state = pstate::scol;
                break;
            }
            // fall through
        case pstate::scol:
            if (*c == ':') state = pstate::slash; else state = pstate::h;
            break;
        case pstate::slash:
            if (*c == '/') state = pstate::slash2; else state = pstate::h;
            break;
        case pstate::slash2:
            if (*c == '/')
            {
                res.second = c + 1;
                return res;
            }
            else state = pstate::h;
        }
    }
    return make_pair(end, end);
}

scit find_domain(scit begin, scit end)
{
    scit res = begin;
    for (scit it = begin; it != end; it++)
    {
        res = it;
        if (!isalnum(*it) && *it != '.' && *it != '-')
            break;
    }
    return res;
}

scit find_path(scit begin, scit end)
{
    scit res = begin;
    for (scit it = begin; it != end; it++)
    {
        res = it;
        if (!isalnum(*it) && *it != '.' && *it != ',' && *it != '/' && *it != '+' && *it != '_')
            break;
    }
    return res;
}

void process_line(const string& line)
{
    scit cur_it = line.begin();
    while (true)
    {
        find_res proto_res = find_proto(cur_it, line.end());
        if (proto_res.first == line.end())
            break;
        cur_it = proto_res.second;
        scit domain_start = cur_it;
        scit domain_end = find_domain(domain_start, line.end());
        if (domain_end == domain_start)
            continue;
        cur_it = domain_end;
        scit path_start = cur_it;
        scit path_end = find_path(path_start, line.end());
        cur_it = path_end;

        string domain = string(domain_start, domain_end);
        string path = path_start == path_end ? "/" : string(path_start, path_end);

        urls_count++;
        inc_or_add(domains, domain);
        inc_or_add(paths, path);
    }
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
            //process_line(line, re);
            process_line(line);
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
