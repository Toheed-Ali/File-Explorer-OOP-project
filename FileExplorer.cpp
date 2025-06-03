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

// Abstract base class for all file system objects
class FileSystemObject 
{
protected:
    string name;
    string path;

public:
    FileSystemObject(const string& name, const string& path) : name(name), path(path) {}
    
    virtual ~FileSystemObject() {}
    
    string getName() const 
    { 
        return name; 
    }
    string getPath() const 
    { 
        return path; 
    }
    
    void setName(const string& newName) 
    { 
        name = newName; 
    }
    void setPath(const string& newPath) 
    { 
        path = newPath; 
    }
    
    string getFullPath() const 
    {
        if (path.empty() || path == "\\") 
        {
            return path + name;
        }
        return path + "\\" + name;
    }
};

class File : public FileSystemObject
{
protected:
    string extension;
    string content;

public:
    File(const string& name, const string& path, const string& extension) : FileSystemObject(name, path), extension(extension), content("") {}
    
    ~File() {}
    
    string getExtension() const { return extension; }
    string getContent() const { return content; }
    void setContent(const string& newContent) { content = newContent; }
    
    void display() const override {
        cout << "📄 " << name << extension << endl;
    }
    
    void viewContent() const {
        cout << "\n===== Content of " << name << extension << " =====\n";
        cout << content << endl;
        cout << "===== End of file =====\n\n";
    }
};

// Directory class representing directories in the file system
class Directory : public FileSystemObject 
{
private:
    vector<FileSystemObject*> contents;

public:
    Directory(const string& name, const string& path) : FileSystemObject(name, path) {}
    
    ~Directory() {
        // Clean up all contained objects
        for (size_t i = 0; i < contents.size(); ++i)
        {
            delete contents[i];
        }
        contents.clear();
    }
};

class FileEditor {
private:
    File* file;
public:
    FileEditor(File* file) : file(file) {}
};

// Main class for file explorer functionality
class FileExplorer {
private:
    Directory* rootDirectory;
    Directory* currentDirectory;
    FileSystemObject* copyBuffer;
public:
    FileExplorer() 
    {
        // Create root directory
        rootDirectory = new Directory("root", "");
        currentDirectory = rootDirectory;
        copyBuffer = nullptr;
    }
    ~FileExplorer()
    {
        delete rootDirectory;  // This will recursively delete all contents
        if (copyBuffer) 
        {
            delete copyBuffer;
        }
    }
};