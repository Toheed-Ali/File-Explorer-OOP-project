#include <iostream>
#include <fstream>
#include "MyString.h"
#include <vector>
using namespace std;

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
    MyString name;
    MyString path;

public:
    FileSystemObject(const MyString& name, const MyString& path) : name(name), path(path) {}

    virtual ~FileSystemObject() {}

    MyString getName() const
    {
        return name;
    }

    MyString getPath() const
    {
        return path;
    }

    void setName(const MyString& newName)
    {
        name = newName;
    }

    void setPath(const MyString& newPath)
    {
        path = newPath;
    }

    MyString getFullPath() const
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
    MyString extension;
    MyString content;

public:
    File(const MyString& name, const MyString& path, const MyString& extension)
        : FileSystemObject(name, path), extension(extension), content("") {}

    ~File() override {}

    MyString getExtension() const
    {
        return extension;
    }

    MyString getContent() const
    {
        return content;
    }

    void setContent(const MyString& newContent)
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
        MyString fullPath = getFullPath() + extension;

        ofstream file(fullPath.toStdString());
        if (file.is_open())
        {
            file << content.toStdString();
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
    static File* createFile(const MyString& name, const MyString& path, const MyString& fullName)
    {
        int dotPos = -1;

        for (int i = fullName.length() - 1; i >= 0; i--)
        {
            if (fullName[i] == '.')
            {
                dotPos = i;
                break;
            }
        }

        if (dotPos != -1)
        {
            MyString ext = fullName.substring(dotPos, fullName.length() - dotPos);
            if (ext == ".txt" || ext == ".cpp")
            {
                return new File(name, path, ext);
            }
        }

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
    Directory(const MyString& name, const MyString& path, Directory* parent = nullptr)
        : FileSystemObject(name, path), parent(parent) {}

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
        MyString newPath = this->getFullPath();
        item->setPath(newPath);
        contents.push_back(item);
    }

    bool removeItem(const MyString& itemName)
    {
        for (int i = 0; i < contents.size(); i++)
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

    FileSystemObject* findItem(const MyString& itemName) const
    {
        for (int i = 0; i < contents.size(); i++)
        {
            if (contents[i]->getName() == itemName)
            {
                return contents[i];
            }
        }

        for (int i = 0; i < contents.size(); i++)
        {
            if (!contents[i]->isDirectory())
            {
                int dotPos = -1;
                for (int j = itemName.length() - 1; j >= 0; j--)
                {
                    if (itemName[j] == '.')
                    {
                        dotPos = j;
                        break;
                    }
                }
                MyString baseName;
                if (dotPos != -1)
                {
                    baseName = itemName.substring(0, dotPos);
                }
                else
                {
                    baseName = itemName;
                }
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
        cout << "\nCurrent path: " << getFullPath().c_str() << endl;
        cout << "\nFiles and folders are:\n";

        int index = 1;
        for (int i = 0; i < contents.size(); i++)
        {
            cout << index << ". ";
            index++;
            contents[i]->display();
        }
        cout << endl;
    }

    void display() const override
    {
        cout << "📁  " << name.c_str() << endl;
    }

    FileSystemObject* clone() const override
    {
        Directory* copy = new Directory(name, path);
        for (int i = 0; i < contents.size(); i++)
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
        for (int i = 0; i < contents.size(); i++)
        {
            contents[i]->saveContentToFile();
        }
    }

    void saveHierarchy(ofstream& file, int depth = 0) const
    {
        string indent(depth * 2, ' ');
        file << indent << "📁 " << name.c_str() << endl;

        for (int i = 0; i < contents.size(); i++)
        {
            if (contents[i]->isDirectory())
            {
                Directory* dir = static_cast<Directory*>(contents[i]);
                dir->saveHierarchy(file, depth + 1);
            }
            else
            {
                file << indent << "  📄 " << contents[i]->getName().c_str() << endl;
            }
        }
    }
};

// File editor for editing file content
class FileEditor 
{
private:
    File* file;

    string readMultilineInput() 
    {
        string NewContent;
        string line;
        cout << "Enter file content (type :w or :save to save, :q or :quit to quit, :q! or :quit! to quit without saving):\n";
        
        while (true)
        {
            getline(cin, line);
            if (line == ":w" || line == ":save") 
            {
                return NewContent; // Return new content
            }
            else if (line == ":q" || line == ":quit")
            {
                cout << "Save changes? (y/n): ";
                char choice;
                cin >> choice;
                cin.ignore();  // Clear the newline
                if (choice == 'y' || choice == 'Y') 
                {
                    return NewContent; // Return new content
                } 
                else 
                {
                    return file->getContent(); // Return original content
                }
            }
            else if (line == ":q!" || line == ":quit!") 
            {
                return file->getContent(); // Return original content without saving
            }
            else
            {
                NewContent += line + "\n";
            }
        }
    }
public:
    FileEditor(File* file) : file(file) {}
    
    void saveChanges(const string& NewContent) 
    {
        file->setContent(NewContent);
    }
    void editContent() 
    {
        cout << "\n===== Editing " << file->getName() << file->getExtension() << " =====\n";
        cout << "Current content:\n";
        cout << file->getContent() << endl;
        
        string newContent = readMultilineInput();
        saveChanges(newContent);
    }
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

// Command handler for processing user commands
class CommandHandler 
{
private:
    FileExplorer& explorer;
    bool running;
public:
}

