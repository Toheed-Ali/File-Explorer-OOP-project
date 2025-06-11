#include <iostream>
#include <string>
#include <vector>
#include <fstream>
using namespace std;

// Forward declarations of classes that will be used
class FileSystemObject;
class File;
class Directory;
class FileExplorer;
class CommandHandler;
class FileEditor;

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
            return name;
        }
        return path + "\\" + name;
    }
    virtual void display() const = 0;
    virtual FileSystemObject* clone() const = 0;
    virtual bool isDirectory() const = 0;
    virtual void saveContentToFile() const = 0;
};

// File class representing files in the file system
class File : public FileSystemObject 
{
private:
    string extension;
    string content;
public:
    File(const string& name, const string& path, const string& extension) : FileSystemObject(name, path), extension(extension), content("") {}
    
    ~File() override {}
    
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
        cout << "=========== End of file ===========" << endl << endl;
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
    void saveContentToFile() const override 
    {
        string fullPath = getFullPath() + extension;
        ofstream file(fullPath);
        if (file.is_open()) 
        {
            file << content;
            file.close();
        }
        else
        {
            cout << "Error: Could not save file " << fullPath << endl;
        }
    }
};

// Factory for creating file objects
class FileFactory 
{
public:
    static File* createFile(const string& name, const string& path, const string& fullName) 
    {
        int dotPos = -1;
        // Loop through the fullName from end to start to find the last dot manually
        for (int i = fullName.length() - 1; i >= 0; i--) 
        {
            if (fullName[i] == '.') 
            {
                dotPos = i;
                break;
            }
        }
        // If a dot was found (dotPos changed from -1)
        if (dotPos != -1) 
        {
            string ext = fullName.substr(dotPos);
            if (ext == ".txt" || ext == ".cpp")
            {
                return new File(name, path, ext);
            }
        }
        // Default to .txt if no valid extension
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

    ~Directory() override
    {
        for (int i = 0; i < contents.size(); i++) 
        {
            delete contents[i];
        }
        contents.clear();
    }
    
    Directory* getParent() const 
    {
        return parent;
    }
    
    void addItem(FileSystemObject* item) 
    {
        string newPath = this->getFullPath();
        item->setPath(newPath);
        contents.push_back(item);
    }
    
    bool removeItem(const string& itemName) 
    {
        for (size_t i = 0; i < contents.size(); i++) 
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
        for (size_t i = 0; i < contents.size(); i++) 
        {
            if (contents[i]->getName() == itemName) 
            {
                return contents[i];
            }
        }
        
        for (size_t i = 0; i < contents.size(); i++) 
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
        for (size_t i = 0; i < contents.size(); i++) 
        {
            cout << index << ". ";
            index++;
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
        for (size_t i = 0; i < contents.size(); i++) 
        {
            copy->addItem(contents[i]->clone());
        }
        return copy;
    }
    
    bool isDirectory() const override 
    {
        return true;
    }
    
    void saveContentToFile() const override 
    {
        for (size_t i = 0; i < contents.size(); i++) 
        {
            contents[i]->saveContentToFile();
        }
    }
    
    void saveHierarchy(ofstream& file, int depth = 0) const 
    {
        string indent(depth * 2, ' ');
        file << indent << "📁 " << name << endl;
        
        for (size_t i = 0; i < contents.size(); i++)
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