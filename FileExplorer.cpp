#include <iostream>
#include <fstream>
#include "MyString.h"
#include "MyVector.h"
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
    File(const MyString& name, const MyString& path, const MyString& extension) : FileSystemObject(name, path), extension(extension), content("") {}

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
        ofstream file(fullPath.c_str());
        if (file.is_open())
        {
            file << content.c_str();
            file.close();
        }
        else
        {
            cout << "Error: Could not save file " << fullPath.c_str() << endl;
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
    Directory(const MyString& name, const MyString& path, Directory* parent = nullptr) : FileSystemObject(name, path), parent(parent) {}

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

    MyString readMultilineInput()
    {
        MyString NewContent;
        string line;
        cout << "Enter file content (type :w or :save to save, :q or :quit to quit, :q! or :quit! to quit without saving):\n";

        while (true)
        {
            getline(cin, line);
            MyString inputLine(line.c_str());

            if (inputLine == ":w" || inputLine == ":save")
            {
                return NewContent;
            }
            else if (inputLine == ":q" || inputLine == ":quit")
            {
                cout << "Save changes? (y/n): ";
                char choice;
                cin >> choice;
                cin.ignore();
                if (choice == 'y' || choice == 'Y')
                {
                    return NewContent;
                }
                else
                {
                    return file->getContent();
                }
            }
            else if (inputLine == ":q!" || inputLine == ":quit!")
            {
                return file->getContent();
            }
            else
            {
                NewContent = NewContent + inputLine + "\n";
            }
        }
    }

public:
    FileEditor(File* file) : file(file) {}

    void saveChanges(const MyString& NewContent)
    {
        file->setContent(NewContent);
    }

    void editContent()
    {
        cout << "\n===== Editing " << file->getName().c_str() << file->getExtension().c_str() << " =====\n";
        cout << "Current content:\n";
        cout << file->getContent().c_str() << endl;

        MyString newContent = readMultilineInput();
        saveChanges(newContent);
    }
};

// Main class for file explorer functionality
class FileExplorer
{
private:
    Directory* rootDirectory;
    Directory* currentDirectory;
    FileSystemObject* copyBuffer;

public:
    FileExplorer()
    {
        rootDirectory = new Directory(MyString("root"), MyString(""));
        currentDirectory = rootDirectory;
        copyBuffer = nullptr;
    }

    ~FileExplorer()
    {
        delete rootDirectory;
        if (copyBuffer)
        {
            delete copyBuffer;
        }
    }

    void initialize()
    {
        Directory* Desktop = new Directory(MyString("Desktop"), rootDirectory->getFullPath(), rootDirectory);
        Directory* Documents = new Directory(MyString("Documents"), rootDirectory->getFullPath(), rootDirectory);
        Directory* Downloads = new Directory(MyString("Downloads"), rootDirectory->getFullPath(), rootDirectory);
        Directory* Pictures = new Directory(MyString("Pictures"), rootDirectory->getFullPath(), rootDirectory);

        File* textFile = new File(MyString("name"), rootDirectory->getFullPath(), MyString(".txt"));
        textFile->setContent(MyString("This is a sample text file."));

        File* cppFile = new File(MyString("hello"), rootDirectory->getFullPath(), MyString(".cpp"));
        cppFile->setContent(MyString("#include <iostream>\nusing namespace std;\nint main1() \n{\n    cout << \"Hello, World!\" << endl;\n    return 0;\n}"));

        File* numbersFile = new File(MyString("numbers"), rootDirectory->getFullPath(), MyString(".txt"));
        numbersFile->setContent(MyString("0321-4567483\n0342-4563452\n0322-1345321\n0321-2233445\n0323-2345543"));

        File* picFile = new File(MyString("vacation"), Desktop->getFullPath(), MyString(".txt"));
        picFile->setContent(MyString("Beach photos from summer vacation"));

        rootDirectory->addItem(Desktop);
        rootDirectory->addItem(Documents);
        rootDirectory->addItem(Downloads);
        rootDirectory->addItem(Pictures);

        Desktop->addItem(textFile);
        Documents->addItem(cppFile);
        Downloads->addItem(numbersFile);
        Pictures->addItem(picFile);
    }

    void displayCurrentDirectory() const
    {
        currentDirectory->displayContents();
    }

    bool navigate(const MyString& dirName)
    {
        if (dirName == "..")
        {
            if (currentDirectory != rootDirectory && currentDirectory->getParent())
            {
                currentDirectory = currentDirectory->getParent();
                return true;
            }
            else if (currentDirectory != rootDirectory)
            {
                currentDirectory = rootDirectory;
                return true;
            }
            return false;
        }
        else
        {
            FileSystemObject* item = currentDirectory->findItem(dirName);
            if (item && item->isDirectory())
            {
                currentDirectory = static_cast<Directory*>(item);
                return true;
            }
            return false;
        }
    }

    bool viewFile(const MyString& fileName)
    {
        FileSystemObject* item = currentDirectory->findItem(fileName);
        if (item && !item->isDirectory())
        {
            File* file = static_cast<File*>(item);
            file->viewContent();
            return true;
        }
        return false;
    }

    bool deleteItem(const MyString& itemName)
    {
        FileSystemObject* item = currentDirectory->findItem(itemName);
        if (!item)
        {
            cout << "Error: Item '" << itemName.c_str() << "' not found." << endl;
            return false;
        }
        cout << "Are you sure you want to delete '" << itemName.c_str() << "'? (y/n): ";
        char choice;
        cin >> choice;
        cin.ignore();
        if (choice == 'y' || choice == 'Y')
        {
            return currentDirectory->removeItem(item->getName());
        }
        return false;
    }

    bool editFile(const MyString& fileName)
    {
        FileSystemObject* item = currentDirectory->findItem(fileName);
        if (item && !item->isDirectory())
        {
            File* file = static_cast<File*>(item);
            FileEditor editor(file);
            editor.editContent();
            return true;
        }
        return false;
    }

    bool copyItem(const MyString& itemName)
    {
        FileSystemObject* item = currentDirectory->findItem(itemName);
        if (item)
        {
            if (copyBuffer)
            {
                delete copyBuffer;
            }
            copyBuffer = item->clone();
            cout << "Copied: " << itemName.c_str() << endl;
            return true;
        }
        return false;
    }

    bool cutItem(const MyString& itemName)
    {
        FileSystemObject* item = currentDirectory->findItem(itemName);
        if (item)
        {
            if (copyBuffer)
            {
                delete copyBuffer;
            }
            copyBuffer = item->clone();
            cout << "Cutted: " << itemName.c_str() << endl;
            return true;
        }
        return false;
    }

    bool pasteItem()
    {
        if (!copyBuffer)
        {
            cout << "Error: Nothing to paste." << endl;
            return false;
        }
        MyString itemName = copyBuffer->getName();
        FileSystemObject* existingItem = currentDirectory->findItem(itemName);

        if (existingItem)
        {
            cout << "'" << itemName.c_str() << "' already exists. Overwrite? (y/n/rename): ";
            string choice;
            getline(cin, choice);

            if (choice == "y" || choice == "Y")
            {
                currentDirectory->removeItem(itemName);
            }
            else if (choice == "rename")
            {
                cout << "Enter new name: ";
                string newName;
                getline(cin, newName);
                copyBuffer->setName(MyString(newName.c_str()));
            }
            else
            {
                return false;
            }
        }
        copyBuffer->setPath(currentDirectory->getFullPath());
        FileSystemObject* pastedItem = copyBuffer->clone();
        currentDirectory->addItem(pastedItem);
        return true;
    }

    bool createDirectory(const MyString& dirName)
    {
        if (currentDirectory->findItem(dirName))
        {
            cout << "Error: An item named '" << dirName.c_str() << "' already exists." << endl;
            return false;
        }
        Directory* newDir = new Directory(dirName, currentDirectory->getFullPath());
        currentDirectory->addItem(newDir);
        cout << "Directory created: " << dirName.c_str() << endl;
        return true;
    }

    bool createFile(const MyString& fileName)
    {
        MyString baseName = fileName;
        MyString extension = ".txt";

        int dotPos = fileName.lastIndexOf('.');
        if (dotPos != -1)
        {
            baseName = fileName.substr(0, dotPos);
            extension = fileName.substr(dotPos, fileName.length() - dotPos);
        }

        if (currentDirectory->findItem(baseName))
        {
            cout << "Error: An item named '" << baseName.c_str() << "' already exists." << endl;
            return false;
        }
        File* newFile = new File(baseName, currentDirectory->getFullPath(), extension);
        currentDirectory->addItem(newFile);
        cout << "File created: " << fileName.c_str() << endl;
        return true;
    }

    void saveHierarchy() const
    {
        ofstream file("hierarchy.txt");
        if (file.is_open())
        {
            rootDirectory->saveHierarchy(file);
            file.close();
            cout << "Hierarchy saved to hierarchy.txt" << endl;
        }
        else
        {
            cerr << "Error: Could not save hierarchy" << endl;
        }
    }

    void saveAllFiles() const
    {
        rootDirectory->saveContentToFile();
        cout << "All files saved" << endl;
    }

    MyString getCurrentPath() const
    {
        return currentDirectory->getFullPath();
    }
};

// Command handler for processing user commands
class CommandHandler
{
private:
    FileExplorer& explorer;
    bool running;

    // Process cd command
    void handleCd(const vector<string>& args)
    {
        if (args.size() < 2)
        {
            cout << "Error: cd command requires a directory name" << endl;
            return;
        }

        string target = args[1];
        if (!explorer.navigate(target))
        {
            // Try to view as a file if navigation fails
            if (!explorer.viewFile(target))
            {
                cout << "Error: '" << target << "' is not a valid directory or file" << endl;
            }
        }
    }

    // Process view command
    void handleView(const vector<string>& args)
    {
        if (args.size() < 2)
        {
            cout << "Error: view command requires a file name" << endl;
            return;
        }

        string fileName = args[1];
        if (!explorer.viewFile(fileName))
        {
            cout << "Error: Could not view file '" << fileName << "'" << endl;
        }
    }

    // Process delete command
    void handleDelete(const vector<string>& args)
    {
        if (args.size() < 2)
        {
            cout << "Error: delete command requires an item name" << endl;
            return;
        }

        string itemName = args[1];
        if (!explorer.deleteItem(itemName))
        {
            cout << "Error: Could not delete '" << itemName << "'" << endl;
        }
    }

    // Process edit command
    void handleEdit(const vector<string>& args)
    {
        if (args.size() < 2)
        {
            cout << "Error: edit command requires a file name" << endl;
            return;
        }

        string fileName = args[1];
        if (!explorer.editFile(fileName))
        {
            cout << "Error: Could not edit file '" << fileName << "'" << endl;
        }
    }

    // Process copy command
    void handleCopy(const vector<string>& args)
    {
        if (args.size() < 2)
        {
            cout << "Error: copy command requires an item name" << endl;
            return;
        }

        string itemName = args[1];
        if (!explorer.copyItem(itemName))
        {
            cout << "Error: Could not copy '" << itemName << "'" << endl;
        }
    }

    void handleCut(const vector<string>& args)
    {
        if (args.size() < 2)
        {
            cout << "Error: cut command requires an item name" << endl;
            return;
        }
        string itemName = args[1];
        if (!explorer.cutItem(itemName))
        {
            cout << "Error: Could not cut '" << itemName << "'" << endl;
        }
        if (!explorer.deleteItem(itemName))
        {
            cout << "Error: Could not delete '" << itemName << "'" << endl;
        }
    }

    // Process paste command
    void handlePaste(const vector<string>& args)
    {
        if (!explorer.pasteItem())
        {
            cout << "Error: Paste operation failed" << endl;
        }
    }

    // New method to process mkdir command
    void handleMkdir(const vector<string>& args)
    {
        if (args.size() < 2)
        {
            cout << "Error: mkdir command requires a directory name" << endl;
            return;
        }

        string dirName = args[1];
        explorer.createDirectory(dirName);
    }

    // New method to process touch command
    void handleTouch(const vector<string>& args)
    {
        if (args.size() < 2)
        {
            cout << "Error: touch command requires a file name" << endl;
            return;
        }

        string fileName = args[1];
        explorer.createFile(fileName);
    }

    // Process exit command
    void handleExit(const vector<string>& args)
    {
        explorer.saveHierarchy();
        explorer.saveAllFiles();
        running = false;
        cout << "Exiting file explorer..." << endl;
    }

    // Process help command
    void handleHelp(const vector<string>& args)
    {
        if (args.size() > 1)
        {
            string command = args[1];
            if (command == "cd")
            {
                cout << "cd <directory_name> - Navigate to a directory\n";
                cout << "cd .. - Navigate to parent directory\n";
                cout << "cd <file_name> - View file content\n";
            }
            else if (command == "view")
            {
                cout << "view <file_name> - Display file content\n";
            }
            else if (command == "delete")
            {
                cout << "delete <name> - Delete a file or directory\n";
            }
            else if (command == "edit")
            {
                cout << "edit <file_name> - Edit file content\n";
                cout << "In editor: :w or :save - Save changes\n";
                cout << "In editor: :q or :quit - Quit and prompt to save\n";
                cout << "In editor: :q! - Quit without saving\n";
            }
            else if (command == "copy")
            {
                cout << "copy <name> - Copy a file or directory\n";
            }
            else if (command == "paste")
            {
                cout << "paste - Paste copied item into current directory\n";
            }
            else if (command == "mkdir")
            {
                cout << "mkdir <name> - Create a new directory\n";
            }
            else if (command == "touch")
            {
                cout << "touch <name.extension> - Create a new file with optional extension\n";
            }
            else if (command == "exit")
            {
                cout << "exit - Save and exit the file explorer\n";
            }
            else if (command == "help")
            {
                cout << "help - Display available commands\n";
                cout << "help <command> - Display detailed help for a command\n";
            }
            else
            {
                cout << "No help available for '" << command << "'\n";
            }
        }
        else
        {
            cout << "\nAvailable commands:\n";
            cout << "  cd <directory> or cd ..\n";
            cout << "  view <file_name>\n";
            cout << "  delete <name>\n";
            cout << "  edit <file_name>\n";
            cout << "  copy <name>\n";
            cout << "  paste\n";
            cout << "  mkdir <directory_name>\n";
            cout << "  touch <file_name.extension>\n";
            cout << "  exit\n";
            cout << "  help [command]\n";
            cout << "\nType 'help <command>' for more details on a specific command.\n";
        }
    }

public:
    CommandHandler(FileExplorer& explorer) : explorer(explorer), running(true) {}

    void processCommand(const string& commandLine)
    {
        vector<string> args = splitString(commandLine, ' ');
        if (args.empty()) return;

        string command = args[0];

        if (command == "cd")
        {
            handleCd(args);
        }
        else if (command == "view")
        {
            handleView(args);
        }
        else if (command == "delete")
        {
            handleDelete(args);
        }
        else if (command == "edit")
        {
            handleEdit(args);
        }
        else if (command == "copy")
        {
            handleCopy(args);
        }
        else if (command == "cut")
        {
            handleCut(args);
        }
        else if (command == "paste")
        {
            handlePaste(args);
        }
        else if (command == "mkdir")
        {
            handleMkdir(args);
        }
        else if (command == "touch")
        {
            handleTouch(args);
        }
        else if (command == "exit")
        {
            handleExit(args);
        }
        else if (command == "help")
        {
            handleHelp(args);
        }
        else
        {
            cout << "Unknown command: " << command << endl;
            cout << "Type 'help' for a list of commands." << endl;
        }

        // Display current directory after each command (except exit)
        if (running)
        {
            explorer.displayCurrentDirectory();
        }
    }

    bool isRunning() const
    {
        return running;
    }
};

int main()
{
    // Create file explorer and command handler
    FileExplorer explorer;
    CommandHandler commandHandler(explorer);

    // Initialize with some test data
    explorer.initialize();

    cout << "===== Virtual File Explorer =====\n";

    // Display initial directory
    explorer.displayCurrentDirectory();

    // Main command loop
    string commandLine;
    while (commandHandler.isRunning())
    {
        cout << explorer.getCurrentPath() << "> ";
        getline(cin, commandLine);
        commandHandler.processCommand(commandLine);
    }

    return 0;
}