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

bool tackS = false;
const array<string, 5> sizeUnits { "B ", "KB", "MB", "GB", "TB" };

struct Entry{
    filesystem::path path;
    uint64_t size;
};

void printUsage(string errorType, string problem)
{
    if(errorType == "badFlag")
        cerr << "ERROR: Unrecognized flag \"" << problem << "\"\n";
    else if(errorType == "badPath")
        cerr << "ERROR: Unrecognized path \"" << problem << "\"\n";
    cout << "Usage: doug [-hs] [path]\n";
    cout << "   -h : show help\n";
    cout << "   -s : sort by size\n";
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
                    else if(argv[arg][charIndex] == 's')
                        tackS = true;
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

void fillEntryVector(vector<Entry>& entries, string path)
{
    for(const auto& file : filesystem::directory_iterator(path))
    {
        if(file.is_directory())
        {
            Entry temp = {file.path(), 0};
            entries.push_back(temp);
        }
    }
}

uint64_t getFileSizes(string path)
{
    uint64_t totalSize = 0;
    for(const auto& file : filesystem::directory_iterator(path))
    {
        if(file.is_regular_file())
            totalSize += filesystem::file_size(file);
    }
    return totalSize;
}

uint64_t sumSubdirectorySizes(vector<Entry>& entries)
{
    uint64_t totalSize = 0;
    for(auto& entry : entries)
    {
        entry.size = getDirectorySize(entry.path);
        totalSize += entry.size;
    }
    return totalSize;
}

bool alphabeticSort(const Entry& e1, const Entry& e2) 
{
    // case insensitive alphabetic sort
    string name1 = e1.path.filename().string();
    string name2 = e2.path.filename().string();
    transform(name1.begin(), name1.end(), name1.begin(), ::tolower);
    transform(name2.begin(), name2.end(), name2.begin(), ::tolower);
    return (name1.compare(name2) < 0);
}

bool sizeSort(const Entry& e1, const Entry& e2)
{
    return (e1.size > e2.size);
}

void printEntry(const Entry& entry)
{
    double roundedFileSize = static_cast<double>(entry.size);
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
    cout << entry.path.string() << "\n";
}

int main(int argc, char** argv)
{
    const int pathIndex = getFlags(argc, argv);
    const string path = (pathIndex == -1) ? 
        filesystem::current_path().string() : 
        static_cast<string>(argv[pathIndex]);
    if (!filesystem::exists(path))
        printUsage("badPath", path);
    vector<Entry> entries;
    fillEntryVector(entries, path);
    uint64_t pathSize = getFileSizes(path);
    pathSize += sumSubdirectorySizes(entries);
    sort(entries.begin(), entries.end(), (tackS) ? sizeSort : alphabeticSort);
    for(const auto& entry : entries)
        printEntry(entry);
    printEntry(Entry{ path, pathSize });
    return 0;
}