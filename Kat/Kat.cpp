/*
* Kat - a take on 'cat'
*
* Compilation:
* clang++ -std=c++17 -O2 Kat.cpp -o kat (with added warnings)
*/

#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;

void printUsage(string errorType)
{
    if(errorType == "badFlag")
        cerr << "ERROR: Unrecognized flag\n";
    else if(errorType == "badFile")
        cerr << "ERROR: Unrecognized file\n";
    else if(errorType == "isDir")
        cerr << "ERROR: Input is directory\n";
    cout << "Usage: cat [-h] file1 [file2..fileN]\n";
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
                        printUsage("");
                    else
                        printUsage("badFlag");
                }
            }
            // return index of first non '-' argument
            else
                return arg;
        }
        return -1;
    }
}

vector<string> getPaths(int argc, char** argv, int pathIndex)
{
    vector<string> paths;
    for(int i = pathIndex; i < argc; i++)
    {
        if (!filesystem::exists(argv[i]))
            printUsage("badFile");
        else if(filesystem::is_directory(argv[i]))
            printUsage("isDir");
        paths.push_back(argv[i]);
    }
    return paths;
}

void printFiles(vector<string>& paths)
{
    for(auto& path : paths)
    {
        ifstream file(path);
        while(file)
        {
            cout << static_cast<char>(file.get());
        }
    }
}

int main(int argc, char** argv)
{
    const int pathIndex = getFlags(argc, argv);
    if(pathIndex == -1)
        printUsage("badFile");
    vector<string> paths = getPaths(argc, argv, pathIndex);
    printFiles(paths);
    return 0;
}