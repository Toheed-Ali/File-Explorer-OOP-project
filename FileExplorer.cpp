#include <iostream>
#include <string>
#include <vector>
#include <fstream>
using namespace std;

// Forward declarations
class FileSystemObject;
class File;
class Directory;
class FileExplorer;
class CommandHandler;
class FileEditor;

string trimString(const string& str) 
{
    size_t first = 0;
    while (first < str.length() && (str[first] == ' ' || str[first] == '\t' || str[first] == '\n' || str[first] == '\r')) 
    {
        ++first;
    }
    if (first == str.length()) 
    {
        return "";
    }
    size_t last = str.length() - 1;
    while (last > first && (str[last] == ' ' || str[last] == '\t' || str[last] == '\n' || str[last] == '\r')) 
    {
        --last;
    }
    return str.substr(first, (last - first + 1));
}

vector<string> splitString(const string& str, char delimiter) 
{
    vector<string> tokens;
    string token;
    for (size_t i = 0; i < str.length(); ++i) 
    {
        if (str[i] == delimiter) 
        {
            tokens.push_back(trimString(token));
            token.clear();
        } 
        else 
        {
            token += str[i];
        }
    }
    if (!token.empty() || !str.empty()) 
    {
        tokens.push_back(trimString(token));
    }
    return tokens;
}