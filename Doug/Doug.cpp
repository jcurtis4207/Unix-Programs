/*
* Doug - a take on 'du'
*
* Compilation:
* clang++ -std=c++17 -O2 Doug.cpp -o doug (with added warnings)
*/

#include <algorithm>
#include <filesystem>
#include <iostream>

using namespace std;

const array<string, 5> sizeUnits { "B ", "KB", "MB", "GB", "TB" };

void printUsage(string errorType, string problem)
{
    if(errorType == "badFlag")
        cerr << "ERROR: Unrecognized flag \"" << problem << "\"\n";
    else if(errorType == "badPath")
        cerr << "ERROR: Unrecognized path \"" << problem << "\"\n";
    cout << "Usage: doug [-h] [path]\n";
    cout << "   -h : show help\n";
    exit((errorType == "") ? 0 : 1);
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
                        printUsage("", "");
                    else
                        printUsage("badFlag", string(1, argv[arg][charIndex]));
                }
            }
            // return index of first non '-' argument
            else
                return arg;
        }
        return -1;
    }
}

uint64_t getDirectorySize(const filesystem::path& path)
{
    uint64_t totalSize = 0;
    for(const auto& file : filesystem::directory_iterator(path))
    {
        if(filesystem::is_directory(file.path()))
            totalSize += getDirectorySize(file.path());
        else if(filesystem::is_symlink(file.path()))
            continue;
        else
            totalSize += filesystem::file_size(file);
    }
    return totalSize;
}

void printDirSize(uint64_t size, const filesystem::path& path)
{
    double roundedFileSize = static_cast<double>(size);
    unsigned long unitIndex = 0;
    for(unsigned long i = 0; i < sizeUnits.size(); i++)
    {
        if(roundedFileSize > 1024.0)
        {
            roundedFileSize /= 1024.0;
            unitIndex++;
        }
        else
            break;
    }
    cout << setw(6) << fixed << setprecision(1) << roundedFileSize;
    cout << sizeUnits[unitIndex] << " ";
    cout << path.string() << "\n";
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

uint64_t getSubdirectorySizes(vector<filesystem::directory_entry>& entries)
{
    uint64_t totalSize = 0;
    for(auto& entry: entries)
    {
        uint64_t currentDirSize = getDirectorySize(entry.path());
        totalSize += currentDirSize;
        printDirSize(currentDirSize, entry.path());
    }
    return totalSize;
}

void getAllSizes(string path)
{
    vector<filesystem::directory_entry> entries;
    uint64_t pathSize = 0;
    for(const auto& file : filesystem::directory_iterator(path))
    {
        if(file.is_symlink())
            continue;
        else if(file.is_directory())
            entries.push_back(file);
        else
            pathSize += filesystem::file_size(file);
    }
    sort(entries.begin(), entries.end(), entrySort);
    pathSize += getSubdirectorySizes(entries);
    printDirSize(pathSize, filesystem::path(path));
}

int main(int argc, char** argv)
{
    const int pathIndex = getFlags(argc, argv);
    const string path = (pathIndex == -1) ? 
        filesystem::current_path().string() : 
        static_cast<string>(argv[pathIndex]);
    if (!filesystem::exists(path))
        printUsage("badPath", path);
    getAllSizes(path);
    return 0;
}