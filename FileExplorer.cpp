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
    // Pure virtual method to be implemented by derived classes
    virtual void display() const = 0;
    // Pure virtual method for copying
    virtual FileSystemObject* clone() const = 0;
    // Pure virtual method to determine if it's a directory
    virtual bool isDirectory() const = 0;
    // Pure virtual method to save content
    virtual void saveToFile() const = 0;
};

// File class representing files in the file system
class File : public FileSystemObject {
private:
    string extension;
    string content;
public:
    File(const string& name, const string& path, const string& extension) : FileSystemObject(name, path), extension(extension), content("") {}
    
    ~File() {}
    
    string getExtension() const 
    {
        return extension;
    }
    string getContent() const
    {
        return content;
    }
    void setContent(const string& newContent) 
    {
        content = newContent;
    }
    
    void display() const override 
    {
        cout << "📄  " << name << extension << endl;
    }
    
    void viewContent() const 
    {
        cout << "\n===== Content of " << name << extension << " =====\n";
        cout << content << endl;
        cout << "===== End of file =====\n\n";
    }
    
    FileSystemObject* clone() const override 
    {
        File* copy = new File(name, path, extension);
        copy->setContent(content);
        return copy;
    }
    
    bool isDirectory() const override 
    {
        return false;
    }
    
    void saveToFile() const override 
    {
        string fullPath = getFullPath() + extension;
        ofstream file(fullPath.c_str());
        if (file.is_open()) 
        {
            file << content;
            file.close();
        }
        else
        {
            cerr << "Error: Could not save file " << fullPath << endl;
        }
    }
};

// Factory for creating file objects
class FileFactory {
public:
    static File* createFile(const string& name, const string& path, const string& fullName) 
    {
        size_t dotPos = fullName.find_last_of('.');
        if (dotPos != string::npos) 
        {
            string ext = fullName.substr(dotPos);
            if (ext == ".txt" || ext == ".cpp") 
            {
                return new File(name, path, ext);
            }
        }
        // Default to txt if no extension or unsupported extension
        return new File(name, path, ".txt");
    }
};

// Directory class representing directories in the file system
class Directory : public FileSystemObject 
{
private:
    vector<FileSystemObject*> contents;
    Directory* parent;

public:
    Directory(const string& name, const string& path, Directory* parent = nullptr) : FileSystemObject(name, path), parent(parent) {}

    Directory* getParent() const 
    {
        return parent;
    
    }

    ~Directory() 
    {
        // Clean up all contained objects
        for (size_t i = 0; i < contents.size(); ++i) 
        {
            delete contents[i];
        }
        contents.clear();
    }
    
    void addItem(FileSystemObject* item) 
    {
        // Update the path of the item to reflect its new location
        string newPath = this->getFullPath();
        item->setPath(newPath);
        contents.push_back(item);
    }
    
    bool removeItem(const string& itemName) 
    {
        for (size_t i = 0; i < contents.size(); ++i) 
        {
            if (contents[i]->getName() == itemName) 
            {
                delete contents[i];
                contents.erase(contents.begin() + i);
                return true;
            }
        }
        return false;
    }
    
    const vector<FileSystemObject*>& getItems() const 
    {
        return contents;
    }
    
    FileSystemObject* findItem(const string& itemName) const 
    {
        // First try exact match
        for (size_t i = 0; i < contents.size(); ++i) 
        {
            if (contents[i]->getName() == itemName) 
            {
                return contents[i];
            }
        }
        
        // Then try with extensions for files
        for (size_t i = 0; i < contents.size(); ++i) 
        {
            if (!contents[i]->isDirectory()) 
            {
                size_t dotPos = itemName.find_last_of('.');
                string baseName = (dotPos != string::npos) ? itemName.substr(0, dotPos) : itemName;

                if (contents[i]->getName() == baseName) 
                {
                    return contents[i];
                }
            }
        }
        
        return nullptr;
    }
    
    void displayContents() const 
    {
        cout << "\nCurrent path: " << getFullPath() << endl;
        cout << "\nFiles and folders are:\n";
        
        int index = 1;
        for (size_t i = 0; i < contents.size(); ++i) 
        {
            cout << index++ << ". ";
            contents[i]->display();
        }
        cout << endl;
    }
    
    void display() const override 
    {
        cout << "📁  " << name << endl;
    }
    
    FileSystemObject* clone() const override 
    {
        Directory* copy = new Directory(name, path);
        for (size_t i = 0; i < contents.size(); ++i) 
        {
            copy->addItem(contents[i]->clone());
        }
        return copy;
    }
    
    bool isDirectory() const override 
    {
        return true;
    }
    
    void saveToFile() const override 
    {
        // Save all contained items
        for (size_t i = 0; i < contents.size(); ++i) 
        {
            contents[i]->saveToFile();
        }
    }
    
    void saveHierarchy(ofstream& file, int depth = 0) const 
    {
        string indent(depth * 2, ' ');
        file << indent << "📁 " << name << endl;
        
        for (size_t i = 0; i < contents.size(); ++i)
         {
            if (contents[i]->isDirectory()) 
            {
                Directory* dir = static_cast<Directory*>(contents[i]);
                dir->saveHierarchy(file, depth + 1);
            } 
            else 
            {
                file << indent << "  📄 " << contents[i]->getName() << endl;
            }
        }
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