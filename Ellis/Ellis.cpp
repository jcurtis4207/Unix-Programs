/*
* Ellis - a take on 'ls'
*
* Compilation:
* clang++ -std=c++17 -O2 Ellis.cpp -o ellis (with added warnings)
*/

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <grp.h>
#include <iostream>
#include <pwd.h>
#include <sstream>
#include <sys/stat.h>
#include <vector>

using namespace std;

#define BOLD string("\033[1;")
#define NORMAL string("\033[0;")
#define BLUE string("34m")
#define MAGENTA string("35m")
#define CYAN string("36m")
#define WHITE string("37m")

struct Entry {
    filesystem::path path;
    bool isDirectory;
    bool isSymlink;
    string perms;
    string owner;
    string size;
    string name;
};

bool tackS = false;
bool tackA = false;
const array<string, 5> sizeUnits { "B ", "KB", "MB", "GB", "TB" };

void getPermissions(Entry& entry)
{
    stringstream output;
    if(entry.isSymlink)
        output << "l";
    else if(entry.isDirectory)
        output << "d";
    else
        output << "-";
    const auto perms = filesystem::status(entry.path).permissions();
    const auto none = filesystem::perms::none;
    output << ((perms & filesystem::perms::owner_read) != none ? "r" : "-");
    output << ((perms & filesystem::perms::owner_write) != none ? "w" : "-");
    output << ((perms & filesystem::perms::owner_exec) != none ? "x" : "-");
    output << ((perms & filesystem::perms::group_read) != none ? "r" : "-");
    output << ((perms & filesystem::perms::group_write) != none ? "w" : "-");
    output << ((perms & filesystem::perms::group_exec) != none ? "x" : "-");
    output << ((perms & filesystem::perms::others_read) != none ? "r" : "-");
    output << ((perms & filesystem::perms::others_write) != none ? "w" : "-");
    output << ((perms & filesystem::perms::others_exec) != none ? "x" : "-");
    entry.perms = output.str();
}

void getOwner(Entry& entry)
{
    struct stat info;
    const auto path = entry.path.c_str();
    if(stat(path, &info) != 0)
        entry.owner = "NULL";
    const struct passwd *pw = getpwuid(info.st_uid);
    const struct group  *gr = getgrgid(info.st_gid);
    stringstream output;
    // get owner name
    if(pw != 0)
        output << pw->pw_name;
    // get group name
    if(gr != 0)
        output << " " << gr->gr_name;
    entry.owner = output.str();
}

void getSize(Entry& entry)
{
    // skip for directories
    if(entry.isDirectory)
    {
        entry.size = "";
        return;
    }
    const auto fileSize = filesystem::file_size(entry.path);
    unsigned long unitIndex = 0;
    stringstream output;
    if(fileSize < 1024)
        output << fileSize;
    else
    {
        double roundedFileSize = static_cast<double>(fileSize);
        while(roundedFileSize > 1024.0 && unitIndex < 5)
        {
            roundedFileSize /= 1024.0;
            unitIndex++;
        }
        output << fixed << setprecision(2) << roundedFileSize; 
    }
    output << sizeUnits[unitIndex];
    entry.size = output.str();
}

void getDirectoryName(Entry& entry)
{
    stringstream output;
    if(tackS)
    {
        output << BOLD + WHITE << entry.path.filename().string() 
            << NORMAL + WHITE;
        entry.name = output.str();
        return;
    }
    if(entry.isSymlink)
    {
        output << BOLD + CYAN << entry.path.filename().string() 
            << NORMAL + WHITE << " -> ";
        output << BOLD + BLUE << filesystem::read_symlink(entry.path) 
            << NORMAL + WHITE;
    }
    else
        output << BOLD + BLUE << entry.path.filename().string() 
            << NORMAL + WHITE;
    entry.name = output.str();
}

void getFileName(Entry& entry)
{
    if(tackS)
    {
        entry.name = entry.path.filename().string();
        return;
    } 
    stringstream output;
    if(entry.isSymlink)
    {
        output << NORMAL + CYAN << entry.path.filename().string() 
            << NORMAL + WHITE << " -> ";
        output << NORMAL + MAGENTA << filesystem::read_symlink(entry.path) 
            << NORMAL + WHITE;
    }
    else
        output << NORMAL + MAGENTA << entry.path.filename().string() 
            << NORMAL + WHITE;
    entry.name = output.str();
}

void populateStructs(vector<Entry>& entries)
{
    for(auto& entry : entries)
    {
        if(!tackS)
        {
            getPermissions(entry);
            getOwner(entry);
            getSize(entry);
        }
        if(entry.isDirectory)
            getDirectoryName(entry);
        else
            getFileName(entry);
    }
}

void printOutput(vector<Entry>& entries)
{
    // print simplified output
    if(tackS)
    {
        for(auto& entry : entries)
        {
            cout << entry.name << "\n";
        }
        return;
    }
    // get column widths
    unsigned long permWidth = 0;
    unsigned long ownerWidth = 0;
    unsigned long sizeWidth = 0;
    for(auto& entry : entries)
    {
        permWidth = max(entry.perms.length(), permWidth);
        ownerWidth = max(entry.owner.length(), ownerWidth);
        sizeWidth = max(entry.size.length(), sizeWidth);
    }
    // print formatted output
    for(auto& entry : entries)
    {
        cout << left << setw(static_cast<int>(permWidth) + 1) << entry.perms;
        cout << left << setw(static_cast<int>(ownerWidth) + 1) << entry.owner;
        cout << right << setw(static_cast<int>(sizeWidth)) << entry.size << " ";
        cout << entry.name << "\n";
    }
}

void printUsage(bool tackH)
{
    if(!tackH)
        cerr << "ERROR: Unrecognized flag\n";
    cout << "Usage: ellis [-ash] [path]\n";
    cout << "   -a : show hidden files\n";
    cout << "   -s : simple - don't show file attributes, colors, or follow symlinks\n";
    cout << "   -h : show help\n";
    exit(tackH ? 0 : 1);
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
                // set flags from arguments beginning with '-'
                for(string::size_type charIndex = 1; 
                    charIndex < string(argv[arg]).length(); charIndex++)
                {
                    if(argv[arg][charIndex] == 'h')
                        printUsage(true);
                    else if(argv[arg][charIndex] == 'a')
                        tackA = true;
                    else if(argv[arg][charIndex] == 's')
                        tackS = true;
                    else
                        printUsage(false);
                }
            }
            // return index of first non '-' argument
            else
                return arg;
        }
        return -1;
    }
}

bool entrySort(const Entry& e1, const Entry& e2) 
{
    // sort directories before files
    if(e1.isDirectory && !e2.isDirectory)
        return true;
    else if(!e1.isDirectory && e2.isDirectory)
        return false;
    // sort by name, case-insensitive
    string name1 = e1.path.filename().string();
    string name2 = e2.path.filename().string();
    transform(name1.begin(), name1.end(), name1.begin(), ::tolower);
    transform(name2.begin(), name2.end(), name2.begin(), ::tolower);
    return (name1.compare(name2) < 0);
}

vector<Entry> getDirectoryContents(string path)
{
    vector<Entry> entries;
    for(const auto& file : filesystem::directory_iterator(path))
    {
        // skip hidden files unless -a specified
        if(!tackA && file.path().filename().string()[0] == '.')
            continue;
        Entry temp;
        temp.path = file.path();
        temp.isDirectory = file.is_directory();
        temp.isSymlink = file.is_symlink();
        entries.push_back(temp);
    }
    sort(entries.begin(), entries.end(), entrySort);
    return entries;
}

int main(int argc, char** argv)
{
    const int pathIndex = getFlags(argc, argv);
    const string path = (pathIndex == -1) ? 
        filesystem::current_path().string() : 
        static_cast<string>(argv[pathIndex]);
    if (!filesystem::exists(path))
    {
        cerr << "ERROR: Path does not exist\n";
        return 1;
    }
    vector<Entry> entries = getDirectoryContents(path);
    populateStructs(entries);
    printOutput(entries);
    return 0;
}