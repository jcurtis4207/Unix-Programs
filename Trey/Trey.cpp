/*
* Trey - a take on 'tree'
*
* Compilation:
* clang++ -std=c++17 -O2 Trey.cpp -o trey (with added warnings)
*/

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <vector>

using namespace std;

bool tackA = false;
bool tackD = false;
unsigned long tackL = 1;
// for each entry, this stores if the entry at each preceding
// level was the last entry in that level, for formatting
vector<bool> lastFlags;

void printUsage(bool badFlag, bool badPath, string problem)
{
    if(badFlag)
        cerr << "ERROR: Unrecognized flag \"" << problem << "\"\n";
    else if(badPath)
        cerr << "ERROR: Unrecognized path \"" << problem << "\"\n";
    cout << "Usage: trey [-adh] [-l n] [path]\n";
    cout << "   -a : show hidden files\n";
    cout << "   -d : show directories only\n";
    cout << "   -h : show help\n";
    cout << "   -l n : set depth to n\n";
    exit((badFlag || badPath) ? 1 : 0);
}

bool isNumeric(string input) {
    for (unsigned long i = 0; i < input.length(); i++)
    {
        if (!isdigit(input[i]))
            return false;
    }
    return true;
}

int setFlags(int arg, char** argv)
{
    for(string::size_type charIndex = 1; 
        charIndex < string(argv[arg]).length(); charIndex++)
    {
        if(argv[arg][charIndex] == 'h')
            printUsage(false, false, "");
        else if(argv[arg][charIndex] == 'a')
            tackA = true;
        else if(argv[arg][charIndex] == 'd')
            tackD = true;
        else if(argv[arg][charIndex] == 'l')
        {
            if(!isNumeric(argv[arg + 1]))
                printUsage(true, false, argv[arg + 1]);
            tackL = static_cast<unsigned long>(stoi(argv[++arg]));
            break;
        }
        else
            printUsage(true, false, string(1, argv[arg][charIndex]));
    }
    return arg;
}

int getFlags(int argc, char** argv)
{
    // if no arguments
    if(argc == 1)
        return -1;
    else
    {
        for(int arg = 1; arg < argc; arg++)
        {
            if(argv[arg][0] == '-')
            {
                arg = setFlags(arg, argv);
            }
            // return index of first non '-' argument
            else
                return arg;
        }
        return -1;
    }
}

bool entrySort(const filesystem::directory_entry& p1, 
    const filesystem::directory_entry& p2) 
{
    // sort directories before files
    if(p1.is_directory() && !p2.is_directory())
        return true;
    else if(!p1.is_directory() && p2.is_directory())
        return false;
    // sort by name, case-insensitive
    string name1 = p1.path().filename().string();
    string name2 = p2.path().filename().string();
    transform(name1.begin(), name1.end(), name1.begin(), ::tolower);
    transform(name2.begin(), name2.end(), name2.begin(), ::tolower);
    return (name1.compare(name2) < 0);
}

void printFormatting(unsigned long depth, bool last)
{
    for(unsigned long level = 0; level < tackL - depth; level++)
    {
        if(lastFlags[level])
            cout << "    ";
        else
            cout << "|   ";
    }
    if(last)
        cout << "`-- ";
    else
        cout << "|-- ";
}

void printDirectoryContents(string path, unsigned long depth)
{
    if(depth < 1)
        return;
    vector<filesystem::directory_entry> entries;
    for(const auto& file : filesystem::directory_iterator(path))
    {
        // skip hidden files unless -a specified
        if(!tackA && file.path().filename().string()[0] == '.')
            continue;
        // skip files if -d specified
        if(tackD && !file.is_directory())
            continue;
        entries.push_back(file);
    }
    sort(entries.begin(), entries.end(), entrySort);
    // recurse through directories
    for(unsigned long i = 0; i < entries.size(); i++)
    {
        const bool last = (i == (entries.size() - 1));
        lastFlags[tackL - depth] = last;
        printFormatting(depth, last);
        cout << entries[i].path().filename().string() << "\n";
        if(entries[i].is_directory())
            printDirectoryContents(entries[i].path().string(), depth - 1);
    }    
}

int main(int argc, char** argv)
{
    const int pathIndex = getFlags(argc, argv);
    const string path = (pathIndex == -1) ? 
        filesystem::current_path().string() : 
        static_cast<string>(argv[pathIndex]);
    if (!filesystem::exists(path))
        printUsage(false, true, path);
    lastFlags.resize(static_cast<unsigned long>(tackL));
    cout << path << "\n";
    printDirectoryContents(path, tackL);
    return 0;
}