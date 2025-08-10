#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <windows.h>
#include <iomanip>
#pragma execution_character_set("utf-8") // for emojis in console
using namespace std;

class FileSystemObject;
class File;
class Directory;
class FileExplorer;
class CommandHandler;
class FileEditor;

void setConsoleColor(WORD color) 
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
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
        setConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN);
        cout << setfill('=') << setw(50) << "" << "\n";
        cout << setfill(' ') << setw(25) << "Content of " << name << extension << "\n";
        cout << setfill('=') << setw(50) << "" << "\n";

        cout << content << endl;
        cout << "================== End of file ===================" << endl;
        setConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
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
        size_t dotPos = fullName.rfind('.');
        if (dotPos != string::npos)
        {
            string ext = fullName.substr(dotPos);
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
    Directory(const string& name, const string& path, Directory* parent = nullptr) : FileSystemObject(name, path), parent(parent) {}
    ~Directory() override
    {
        for (auto item : contents)
        {
            delete item;
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
        for (auto item : contents)
        {
            if (item->getName() == itemName)
            {
                return item;
            }
        }
        for (auto item : contents)
        {
            if (!item->isDirectory())
            {
                size_t dotPos = itemName.rfind('.');
                string baseName;
                if (dotPos != string::npos)
                {
                    baseName = itemName.substr(0, dotPos);
                }
                else
                {
                    baseName = itemName;
                }
                if (item->getName() == baseName)
                {
                    return item;
                }
            }
        }
        return nullptr;
    }
    void displayContents() const
    {
        cout << "\nFiles and folders are:\n";
        int index = 1;
        for (auto item : contents)
        {
            cout << index << ". ";
            index++;
            item->display();
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
        for (auto item : contents)
        {
            copy->addItem(item->clone());
        }
        return copy;
    }
    bool isDirectory() const override
    {
        return true;
    }
    void saveContentToFile() const override
    {
        for (auto item : contents)
        {
            item->saveContentToFile();
        }
    }
    void saveHierarchy(ofstream& file, int depth = 0) const
    {
        string indent(depth * 2, ' ');
        file << indent << "📁 " << name << endl;
        for (auto item : contents)
        {
            if (item->isDirectory())
            {
                Directory* dir = static_cast<Directory*>(item);
                dir->saveHierarchy(file, depth + 1);
            }
            else
            {
                file << indent << "  📄 " << item->getName() << endl;
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
        string newContent;
        string line;
        cout << "Enter file content (type :w or :save to save, :q or :quit to quit, :q! or :quit! to quit without saving):\n";

        while (true)
        {
            setConsoleColor(FOREGROUND_GREEN | FOREGROUND_BLUE);
            getline(cin, line);
            setConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            if (line == ":w" || line == ":save")
            {
                return newContent;
            }
            else if (line == ":q" || line == ":quit")
            {
                cout << "Save changes? (y/n): ";
                char choice;
                cin >> choice;
                cin.ignore();
                if (choice == 'y' || choice == 'Y')
                {
                    return newContent;
                }
                else
                {
                    return file->getContent();
                }
            }
            else if (line == ":q!" || line == ":quit!")
            {
                return file->getContent();
            }
            else
            {
                newContent += line + "\n";
            }
        }
    }
public:
    FileEditor(File* file) : file(file) {}

    void saveChanges(const string& newContent)
    {
        file->setContent(newContent);
    }
    void editContent()
    {
        setConsoleColor(FOREGROUND_RED);
        cout << endl << "Editing " << file->getName() << file->getExtension() << endl;
        setConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN);
        cout << setfill('=') << setw(50) << "" << "\n";
        cout << setfill(' ') << setw(30) << "Current Content of " << file->getName() << file->getExtension() << "\n";
        cout << setfill('=') << setw(50) << "" << "\n";

        cout << file->getContent() << endl;
        cout << "================== End of file ===================" << endl;
        setConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

        string newContent = readMultilineInput();
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
        rootDirectory = new Directory("root", "");
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
        Directory* Desktop = new Directory("Desktop", rootDirectory->getFullPath(), rootDirectory);
        Directory* Documents = new Directory("Documents", rootDirectory->getFullPath(), rootDirectory);
        Directory* Downloads = new Directory("Downloads", rootDirectory->getFullPath(), rootDirectory);
        Directory* Pictures = new Directory("Pictures", rootDirectory->getFullPath(), rootDirectory);

        File* textFile = new File("name", rootDirectory->getFullPath(), ".txt");
        textFile->setContent("Toheed Ali\nTalha Malik\nSaad Hamid\nYousaf\nSubhan");

        File* cppFile = new File("hello", rootDirectory->getFullPath(), ".cpp");
        cppFile->setContent("#include <iostream>\nusing namespace std;\nint main1() \n{\n    cout << \"Hello, World!\" << endl;\n    return 0;\n}");

        File* numbersFile = new File("numbers", rootDirectory->getFullPath(), ".txt");
        numbersFile->setContent("0321-4567483\n0342-4563452\n0322-1345321\n0321-2233445\n0323-2345543");

        File* picFile = new File("vacation", Desktop->getFullPath(), ".txt");
        picFile->setContent("Beach photos from summer vacation");

        rootDirectory->addItem(Desktop);
        rootDirectory->addItem(Documents);
        rootDirectory->addItem(Downloads);
        rootDirectory->addItem(Pictures);

        Desktop->addItem(textFile);
        Documents->addItem(cppFile);
        Downloads->addItem(numbersFile);
        Documents->addItem(picFile);
    }

    void displayCurrentDirectory() const
    {
        currentDirectory->displayContents();
    }

    bool navigate(const string& dirName)
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

    bool viewFile(const string& fileName)
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

    bool deleteItem(const string& itemName)
    {
        FileSystemObject* item = currentDirectory->findItem(itemName);
        if (!item)
        {
            cout << "Error: Item '" << itemName << "' not found." << endl;
            return false;
        }
        cout << "Are you sure you want to delete '" << itemName << "'? (y/n): ";
        char choice;
        cin >> choice;
        cin.ignore();
        if (choice == 'y' || choice == 'Y')
        {
            return currentDirectory->removeItem(item->getName());
        }
        return false;
    }

    bool editFile(const string& fileName)
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

    bool copyItem(const string& itemName)
    {
        FileSystemObject* item = currentDirectory->findItem(itemName);
        if (item)
        {
            if (copyBuffer)
            {
                delete copyBuffer;
            }
            copyBuffer = item->clone();
            cout << "Copied: " << itemName << endl;
            return true;
        }
        return false;
    }

    bool cutItem(const string& itemName)
    {
        FileSystemObject* item = currentDirectory->findItem(itemName);
        if (item)
        {
            if (copyBuffer)
            {
                delete copyBuffer;
            }
            copyBuffer = item->clone();
            cout << "Cut: " << itemName << endl;
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
        string itemName = copyBuffer->getName();
        FileSystemObject* existingItem = currentDirectory->findItem(itemName);

        if (existingItem)
        {
            cout << "'" << itemName << "' already exists. Overwrite? (y/n/rename): ";
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
                copyBuffer->setName(newName);
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

    bool createDirectory(const string& dirName)
    {
        if (currentDirectory->findItem(dirName))
        {
            cout << "Error: An item named '" << dirName << "' already exists." << endl;
            return false;
        }
        Directory* newDir = new Directory(dirName, currentDirectory->getFullPath());
        currentDirectory->addItem(newDir);
        cout << "Directory created: " << dirName << endl;
        return true;
    }

    bool createFile(const string& fileName)
    {
        string baseName = fileName;
        string extension = ".txt";
        size_t dotPos = fileName.rfind('.');
        if (dotPos != string::npos)
        {
            baseName = fileName.substr(0, dotPos);
            extension = fileName.substr(dotPos);
        }
        if (currentDirectory->findItem(baseName))
        {
            cout << "Error: An item named '" << baseName << "' already exists." << endl;
            return false;
        }
        File* newFile = new File(baseName, currentDirectory->getFullPath(), extension);
        currentDirectory->addItem(newFile);
        cout << "File created: " << fileName << endl;
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

    string getCurrentPath() const
    {
        return currentDirectory->getFullPath();
    }
};

// Helper function to split string by delimiter
vector<string> splitString(const string& s, char delimiter) {
    vector<string> tokens;
    string token;

    for (char c : s) {
        if (c == delimiter)
        {
            tokens.push_back(token);
            token.clear();
        }
        else
        {
            token += c;
        }
    }
    tokens.push_back(token);
    return tokens;
}

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
        if (args.empty()) 
        {
            return;
        }
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
    // for emojis in console
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    FileExplorer explorer;
    CommandHandler commandHandler(explorer);
    explorer.initialize();
    cout << "===== Virtual File Explorer =====\n";
    explorer.displayCurrentDirectory();
    string commandLine;
    while (commandHandler.isRunning())
    {
        setConsoleColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY); // Light Green
        cout << explorer.getCurrentPath() << "> ";
        setConsoleColor(FOREGROUND_RED);
        getline(cin, commandLine);
        setConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        commandHandler.processCommand(commandLine);
    }
    return 0;
}
