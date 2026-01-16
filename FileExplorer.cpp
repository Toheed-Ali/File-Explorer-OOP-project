#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <cstdlib>
#include <filesystem>
#include <algorithm>
#include <chrono>
#include <sstream>

namespace fs = std::filesystem;
using namespace std;

// Cross-platform console color codes
#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <direct.h>
    #include <conio.h>
    
    void setConsoleColor(int color) {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, color);
    }
    #define COLOR_RESET 7
    #define COLOR_GREEN 10
    #define COLOR_RED 12
    #define COLOR_YELLOW 14
    #define COLOR_CYAN 11
    
    void setupConsole() {
        SetConsoleOutputCP(CP_UTF8);
    }
    void clearScreen() {
        system("cls");
    }
#else
    // ANSI color codes for Linux/Unix
    void setConsoleColor(int color) {
        switch(color) {
            case 10: cout << "\033[1;32m"; break; // Green
            case 12: cout << "\033[1;31m"; break; // Red
            case 14: cout << "\033[1;33m"; break; // Yellow
            case 11: cout << "\033[1;36m"; break; // Cyan
            case 15: cout << "\033[0m"; break;    // Reset
            default: cout << "\033[0m"; break;
        }
    }
    #define COLOR_RESET 15
    #define COLOR_GREEN 10
    #define COLOR_RED 12
    #define COLOR_YELLOW 14
    #define COLOR_CYAN 11
    
    void setupConsole() {
        // UTF-8 should work by default on Linux
    }
    void clearScreen() {
        system("clear");
    }
#endif

// Helper function to draw centered ASCII box header
void drawBoxHeader(const string& title) {
    const int boxWidth = 65;
    int padding = (boxWidth - title.length()) / 2;
    int rightPadding = boxWidth - title.length() - padding;
    
    setConsoleColor(COLOR_CYAN);
    cout << "╔═══════════════════════════════════════════════════════════════════╗\n";
    cout << "║" << string(padding, ' ') << title << string(rightPadding, ' ') << "  ║\n";
    cout << "╚═══════════════════════════════════════════════════════════════════╝\n";
    setConsoleColor(COLOR_RESET);
}

// File Explorer class
class FileExplorer {
private:
    fs::path currentPath;
    fs::path copiedPath;
    bool isCut = false;
    
public:
    FileExplorer() {
        // Start in the user's home directory
        #ifdef _WIN32
        const char* userProfile = getenv("USERPROFILE");
        if (userProfile) {
            currentPath = fs::path(userProfile);
        } else {
            currentPath = fs::current_path();
        }
        #else
        const char* home = getenv("HOME");
        if (home) {
            currentPath = fs::path(home);
        } else {
            currentPath = fs::current_path();
        }
        #endif
    }
    
    // Helper function to format file size in human-readable format
    string formatFileSize(uintmax_t size) const {
        const char* units[] = {"B", "KB", "MB", "GB", "TB"};
        int unitIndex = 0;
        double displaySize = static_cast<double>(size);
        
        while (displaySize >= 1024.0 && unitIndex < 4) {
            displaySize /= 1024.0;
            unitIndex++;
        }
        
        stringstream ss;
        ss << fixed << setprecision(1) << displaySize << " " << units[unitIndex];
        return ss.str();
    }
    
    // Helper function to get file permissions (simplified for Windows)
    string getPermissions(const fs::path& path) const {
        string perms = "";
        
        if (fs::is_directory(path)) {
            perms += "d";
        } else {
            perms += "-";
        }
        
        #ifdef _WIN32
        DWORD attrs = GetFileAttributesW(path.c_str());
        if (attrs != INVALID_FILE_ATTRIBUTES) {
            if (attrs & FILE_ATTRIBUTE_READONLY) {
                perms += "r--r--r--";
            } else {
                perms += "rw-rw-rw-";
            }
        } else {
            perms += "rw-rw-rw-";
        }
        #else
        perms += "rw-rw-rw-"; // Simplified for non-Windows
        #endif
        
        return perms;
    }
    
    // Helper function to format file time
    string formatFileTime(const fs::file_time_type& ftime) const {
        auto sctp = chrono::time_point_cast<chrono::system_clock::duration>(
            ftime - fs::file_time_type::clock::now() + chrono::system_clock::now()
        );
        auto time = chrono::system_clock::to_time_t(sctp);
        char buffer[80];
        strftime(buffer, sizeof(buffer), "%b %d %H:%M", localtime(&time));
        return string(buffer);
    }
    
    // Advanced listing function with flags
    void listDirectory(bool showHidden = false, bool longFormat = false, bool dirsOnly = false, bool recursive = false, const fs::path& dirPath = fs::path(), int depth = 0) const {
        fs::path targetPath = dirPath.empty() ? currentPath : dirPath;
        
        if (depth == 0) {
            if (!longFormat) {
                cout << "\nFiles and folders in: " << targetPath.string() << "\n";
            }
        } else {
            cout << "\n" << targetPath.string() << ":\n";
        }
        
        int index = 1;
        
        try {
            vector<fs::directory_entry> entries;
            
            // Collect all entries
            for (const auto& entry : fs::directory_iterator(targetPath)) {
                string filename = entry.path().filename().string();
                
                // Skip hidden files if not showing all
                if (!showHidden) {
                    bool isHidden = false;
                    
                    #ifdef _WIN32
                    // On Windows, check file attributes for hidden flag
                    DWORD attrs = GetFileAttributesW(entry.path().c_str());
                    if (attrs != INVALID_FILE_ATTRIBUTES) {
                        if (attrs & FILE_ATTRIBUTE_HIDDEN || attrs & FILE_ATTRIBUTE_SYSTEM) {
                            isHidden = true;
                        }
                    }
                    #else
                    // On Linux/Unix, files starting with . are hidden
                    if (!filename.empty() && filename[0] == '.') {
                        isHidden = true;
                    }
                    #endif
                    
                    if (isHidden) {
                        continue;
                    }
                }
                
                // Skip non-directories if only showing directories
                if (dirsOnly && !entry.is_directory()) {
                    continue;
                }
                
                entries.push_back(entry);
            }
            
            // Sort entries alphabetically
            sort(entries.begin(), entries.end(), [](const fs::directory_entry& a, const fs::directory_entry& b) {
                return a.path().filename().string() < b.path().filename().string();
            });
            
            // Display entries
            for (const auto& entry : entries) {
                if (longFormat) {
                    // Long format: permissions, size, date, name
                    string perms = getPermissions(entry.path());
                    auto ftime = fs::last_write_time(entry.path());
                    string timeStr = formatFileTime(ftime);
                    
                    if (entry.is_directory()) {
                        setConsoleColor(COLOR_CYAN);
                        cout << perms << "  " << setw(10) << right << "<DIR>" << "  " 
                             << timeStr << "  " << entry.path().filename().string() << endl;
                        setConsoleColor(COLOR_RESET);
                    } else {
                        auto fileSize = fs::file_size(entry.path());
                        setConsoleColor(COLOR_GREEN);
                        cout << perms << "  " << setw(10) << right << formatFileSize(fileSize) << "  " 
                             << timeStr << "  " << entry.path().filename().string() << endl;
                        setConsoleColor(COLOR_RESET);
                    }
                } else {
                    // Short format with index
                    cout << setfill(' ') << setw(2) << right << index << ". ";
                    if (entry.is_directory()) {
                        setConsoleColor(COLOR_CYAN);
                        cout << "📁  " << entry.path().filename().string() << endl;
                        setConsoleColor(COLOR_RESET);
                    } else {
                        setConsoleColor(COLOR_GREEN);
                        auto fileSize = fs::file_size(entry.path());
                        cout << "📄  " << entry.path().filename().string();
                        cout << " (" << formatFileSize(fileSize) << ")" << endl;
                        setConsoleColor(COLOR_RESET);
                    }
                }
                index++;
            }
            
            // Recursive listing
            if (recursive) {
                for (const auto& entry : entries) {
                    if (entry.is_directory()) {
                        listDirectory(showHidden, longFormat, dirsOnly, recursive, entry.path(), depth + 1);
                    }
                }
            }
            
        } catch (const fs::filesystem_error& e) {
            setConsoleColor(COLOR_RED);
            cout << "Error accessing directory: " << e.what() << endl;
            setConsoleColor(COLOR_RESET);
        }
        
        if (depth == 0) {
            cout << endl;
        }
    }
    
    // Backward compatibility wrapper
    void displayCurrentDirectory() const {
        listDirectory(false, false, false, false);
    }
    
    bool navigate(const string& dirName) {
        fs::path newPath;
        
        // Handle special case: staying in current directory
        if (dirName == ".") {
            return true;
        }
        
        if (dirName == "..") {
            newPath = currentPath.parent_path();
        } else if (dirName == "~") {
            // Navigate to user home directory
            #ifdef _WIN32
            const char* userProfile = getenv("USERPROFILE");
            if (userProfile) {
                newPath = fs::path(userProfile);
            } else {
                return false;
            }
            #else
            const char* home = getenv("HOME");
            if (home) {
                newPath = fs::path(home);
            } else {
                return false;
            }
            #endif
        } else {
            // Check if path contains separators (multi-level path like "codes/portfolio" or "../..")
            bool isMultiLevel = (dirName.find('/') != string::npos || dirName.find('\\') != string::npos);
            
            if (isMultiLevel) {
                // Split path into components
                vector<string> components;
                string component;
                for (char c : dirName) {
                    if (c == '/' || c == '\\') {
                        if (!component.empty()) {
                            components.push_back(component);
                            component.clear();
                        }
                    } else {
                        component += c;
                    }
                }
                if (!component.empty()) {
                    components.push_back(component);
                }
                
                // Navigate through each component with case-correction
                newPath = currentPath;
                for (const string& comp : components) {
                    if (comp == "..") {
                        // Move to parent directory
                        newPath = newPath.parent_path();
                    } else if (comp == ".") {
                        // Stay in current directory
                        continue;
                    } else {
                        // Regular directory name
                        fs::path tempPath = newPath / comp;
                        
                        #ifdef _WIN32
                        // Find correct casing for this component
                        if (fs::exists(tempPath) && fs::is_directory(tempPath)) {
                            try {
                                for (const auto& entry : fs::directory_iterator(newPath)) {
                                    if (entry.is_directory()) {
                                        string entryName = entry.path().filename().string();
                                        string searchName = comp;
                                        
                                        // Case-insensitive comparison
                                        transform(entryName.begin(), entryName.end(), entryName.begin(), ::tolower);
                                        transform(searchName.begin(), searchName.end(), searchName.begin(), ::tolower);
                                        
                                        if (entryName == searchName) {
                                            tempPath = entry.path();
                                            break;
                                        }
                                    }
                                }
                            } catch (const fs::filesystem_error&) {
                                // Continue with tempPath as-is
                            }
                        }
                        #endif
                        
                        // Check if this component exists
                        if (!fs::exists(tempPath) || !fs::is_directory(tempPath)) {
                            return false;
                        }
                        
                        newPath = tempPath;
                    }
                }
            } else {
                // Single directory name - original logic
                newPath = currentPath / dirName;
                
                // On Windows, find the actual directory with correct casing
                #ifdef _WIN32
                if (fs::exists(newPath) && fs::is_directory(newPath)) {
                    try {
                        for (const auto& entry : fs::directory_iterator(currentPath)) {
                            if (entry.is_directory()) {
                                string entryName = entry.path().filename().string();
                                string searchName = dirName;
                                
                                // Case-insensitive comparison
                                transform(entryName.begin(), entryName.end(), entryName.begin(), ::tolower);
                                transform(searchName.begin(), searchName.end(), searchName.begin(), ::tolower);
                                
                                if (entryName == searchName) {
                                    newPath = entry.path();
                                    break;
                                }
                            }
                        }
                    } catch (const fs::filesystem_error&) {
                        // If we can't iterate, just use the path as-is
                    }
                }
                #endif
            }
        }
        
        if (fs::exists(newPath) && fs::is_directory(newPath)) {
            currentPath = newPath;
            return true;
        }
        return false;
    }
    
    bool viewFile(const string& fileName) {
        fs::path filePath = currentPath / fileName;
        
        if (fs::exists(filePath) && fs::is_regular_file(filePath)) {
            // Ask user for choice
            cout << "Choose action:\n";
            cout << "1. View in console\n";
            cout << "2. Open with system app\n";
            cout << "Choice: ";
            
            char choice;
            #ifdef _WIN32
            choice = _getch();
            cout << choice << endl;
            #else
            cin >> choice;
            cin.ignore();
            #endif
            
            string choiceStr(1, choice);
            
            if (choiceStr == "1") {
                // View in console with line numbers
                ifstream file(filePath);
                if (!file.is_open()) {
                    setConsoleColor(COLOR_RED);
                    cout << "Error: Could not open file for reading" << endl;
                    setConsoleColor(COLOR_RESET);
                    return false;
                }
                
                // Draw header box
                const int boxWidth = 80;
                string title = "Content: " + fileName;
                int padding = (boxWidth - title.length()) / 2;
                int rightPadding = boxWidth - title.length() - padding;
                
                cout << "\n";
                setConsoleColor(COLOR_CYAN);
                string topLine(boxWidth, ' ');
                for (int i = 0; i < boxWidth; i++) topLine[i] = '=';
                cout << "╔";
                for (int i = 0; i < boxWidth; i++) cout << "═";
                cout << "╗\n";
                cout << "║" << string(padding, ' ') << title << string(rightPadding, ' ') << "║\n";
                cout << "╚";
                for (int i = 0; i < boxWidth; i++) cout << "═";
                cout << "╝\n";
                setConsoleColor(COLOR_RESET);
                
                // Display file content with line numbers
                string line;
                int lineNum = 1;
                while (getline(file, line)) {
                    setConsoleColor(COLOR_YELLOW);
                    cout << setw(4) << right << lineNum;
                    setConsoleColor(COLOR_CYAN);
                    cout << " │ ";
                    setConsoleColor(COLOR_RESET);
                    cout << line << "\n";
                    lineNum++;
                }
                
                file.close();
                
                // Draw footer box
                setConsoleColor(COLOR_CYAN);
                for (int i = 0; i < boxWidth + 2; i++) cout << "═";
                setConsoleColor(COLOR_RESET);
                cout << "\n";
                
            } else if (choiceStr == "2") {
                // Open with system application - show "Open with" dialog
                setConsoleColor(COLOR_GREEN);
                cout << "Opening 'Open with' dialog for " << fileName << "..." << endl;
                setConsoleColor(COLOR_RESET);
                
                #ifdef _WIN32
                // Windows: Use rundll32 to force "Open with" dialog
                // Get absolute path
                fs::path absPath = fs::absolute(filePath);
                string pathStr = absPath.string();
                
                // Replace backslashes with forward slashes for rundll32
                for (char& c : pathStr) {
                    if (c == '/') c = '\\';
                }
                
                string command = "rundll32.exe shell32.dll,OpenAs_RunDLL " + pathStr;
                system(command.c_str());
                #else
                // Linux: Try to show app chooser if available, otherwise use xdg-open
                string command = "xdg-open \"" + filePath.string() + "\" &";
                system(command.c_str());
                #endif
            } else {
                cout << "Invalid choice. Operation cancelled." << endl;
                return false;
            }
            
            return true;
        }
        return false;
    }
    
    bool editFile(const string& fileName) {
        fs::path filePath = currentPath / fileName;
        
        if (fs::exists(filePath) && fs::is_regular_file(filePath)) {
            // Ask user for choice
            cout << "Choose action:\n";
            cout << "1. Edit in text editor\n";
            cout << "2. Open with system app\n";
            cout << "Choice: ";
            
            char choice;
            #ifdef _WIN32
            choice = _getch();
            cout << choice << endl;
            #else
            cin >> choice;
            cin.ignore();
            #endif
            
            string choiceStr(1, choice);
            
            if (choiceStr == "1") {
                // Edit with system text editor
                setConsoleColor(COLOR_GREEN);
                cout << "Opening " << fileName << " in text editor..." << endl;
                setConsoleColor(COLOR_RESET);
                
                #ifdef _WIN32
                // Windows: Use notepad
                string command = "notepad \"" + filePath.string() + "\"";
                system(command.c_str());
                #else
                // Linux: Use nano
                string command = "nano \"" + filePath.string() + "\"";
                system(command.c_str());
                #endif
                
            } else if (choiceStr == "2") {
                // Open with system application - show "Open with" dialog
                setConsoleColor(COLOR_GREEN);
                cout << "Opening 'Open with' dialog for " << fileName << "..." << endl;
                setConsoleColor(COLOR_RESET);
                
                #ifdef _WIN32
                // Windows: Use rundll32 to force "Open with" dialog
                fs::path absPath = fs::absolute(filePath);
                string pathStr = absPath.string();
                
                // Ensure backslashes
                for (char& c : pathStr) {
                    if (c == '/') c = '\\';
                }
                
                string command = "rundll32.exe shell32.dll,OpenAs_RunDLL " + pathStr;
                system(command.c_str());
                #else
                // Linux: Use xdg-open
                string command = "xdg-open \"" + filePath.string() + "\" &";
                system(command.c_str());
                #endif
            } else {
                cout << "Invalid choice. Operation cancelled." << endl;
                return false;
            }
            
            return true;
        }
        return false;
    }
    
    bool deleteItem(const string& itemName) {
        fs::path itemPath = currentPath / itemName;
        
        if (!fs::exists(itemPath)) {
            cout << "Error: Item '" << itemName << "' not found." << endl;
            return false;
        }
        
        cout << "Are you sure you want to delete '" << itemName << "'? (y/n): ";
        char choice;
        #ifdef _WIN32
        choice = _getch();
        cout << choice << endl;
        #else
        cin >> choice;
        cin.ignore();
        #endif
        
        if (choice == 'y' || choice == 'Y') {
            try {
                if (fs::is_directory(itemPath)) {
                    fs::remove_all(itemPath);
                } else {
                    fs::remove(itemPath);
                }
                setConsoleColor(COLOR_GREEN);
                cout << "Deleted: " << itemName << endl;
                setConsoleColor(COLOR_RESET);
                return true;
            } catch (const fs::filesystem_error& e) {
                setConsoleColor(COLOR_RED);
                cout << "Error deleting item: " << e.what() << endl;
                setConsoleColor(COLOR_RESET);
                return false;
            }
        }
        return false;
    }
    
    bool copyItem(const string& itemName) {
        fs::path itemPath = currentPath / itemName;
        
        if (fs::exists(itemPath)) {
            copiedPath = itemPath;
            isCut = false;
            setConsoleColor(COLOR_GREEN);
            cout << "Copied: " << itemName << endl;
            setConsoleColor(COLOR_RESET);
            return true;
        }
        return false;
    }
    
    bool cutItem(const string& itemName) {
        fs::path itemPath = currentPath / itemName;
        
        if (fs::exists(itemPath)) {
            copiedPath = itemPath;
            isCut = true;
            setConsoleColor(COLOR_GREEN);
            cout << "Cut: " << itemName << endl;
            setConsoleColor(COLOR_RESET);
            return true;
        }
        return false;
    }
    
    bool pasteItem() {
        if (copiedPath.empty()) {
            cout << "Error: Nothing to paste." << endl;
            return false;
        }
        
        fs::path destPath = currentPath / copiedPath.filename();
        
        try {
            if (fs::exists(destPath)) {
                cout << "'" << copiedPath.filename().string() << "' already exists. Overwrite? (y/n): ";
                char choice;
                #ifdef _WIN32
                choice = _getch();
                cout << choice << endl;
                #else
                cin >> choice;
                cin.ignore();
                #endif
                
                if (choice != 'y' && choice != 'Y') {
                    return false;
                }
            }
            
            if (isCut) {
                fs::rename(copiedPath, destPath);
                setConsoleColor(COLOR_GREEN);
                cout << "Moved: " << copiedPath.filename().string() << endl;
                setConsoleColor(COLOR_RESET);
                copiedPath.clear();
            } else {
                if (fs::is_directory(copiedPath)) {
                    fs::copy(copiedPath, destPath, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
                } else {
                    fs::copy(copiedPath, destPath, fs::copy_options::overwrite_existing);
                }
                setConsoleColor(COLOR_GREEN);
                cout << "Pasted: " << copiedPath.filename().string() << endl;
                setConsoleColor(COLOR_RESET);
            }
            return true;
        } catch (const fs::filesystem_error& e) {
            setConsoleColor(COLOR_RED);
            cout << "Error pasting item: " << e.what() << endl;
            setConsoleColor(COLOR_RESET);
            return false;
        }
    }
    
    bool createDirectory(const string& dirName) {
        fs::path newDirPath = currentPath / dirName;
        
        if (fs::exists(newDirPath)) {
            cout << "Error: '" << dirName << "' already exists." << endl;
            return false;
        }
        
        try {
            fs::create_directory(newDirPath);
            setConsoleColor(COLOR_GREEN);
            cout << "Directory created: " << dirName << endl;
            setConsoleColor(COLOR_RESET);
            return true;
        } catch (const fs::filesystem_error& e) {
            setConsoleColor(COLOR_RED);
            cout << "Error creating directory: " << e.what() << endl;
            setConsoleColor(COLOR_RESET);
            return false;
        }
    }
    
    bool createFile(const string& fileName) {
        string finalFileName = fileName;
        
        // If no extension is provided, add .txt
        if (fileName.find('.') == string::npos) {
            finalFileName = fileName + ".txt";
        }
        
        fs::path newFilePath = currentPath / finalFileName;
        
        if (fs::exists(newFilePath)) {
            cout << "Error: '" << finalFileName << "' already exists." << endl;
            return false;
        }
        
        try {
            ofstream file(newFilePath);
            if (file.is_open()) {
                file.close();
                setConsoleColor(COLOR_GREEN);
                cout << "File created: " << finalFileName << endl;
                setConsoleColor(COLOR_RESET);
                return true;
            }
            return false;
        } catch (const exception& e) {
            setConsoleColor(COLOR_RED);
            cout << "Error creating file: " << e.what() << endl;
            setConsoleColor(COLOR_RESET);
            return false;
        }
    }
    
    string getCurrentPath() const {
        // Get the path string and normalize separators to backslashes (Windows style)
        string pathStr = currentPath.string();
        
        #ifdef _WIN32
        // Replace forward slashes with backslashes for Windows
        for (char& c : pathStr) {
            if (c == '/') {
                c = '\\';
            }
        }
        #endif
        
        return pathStr;
    }
};

// Helper function to split string
vector<string> splitString(const string& s, char delimiter) {
    vector<string> tokens;
    string token;
    
    for (char c : s) {
        if (c == delimiter) {
            tokens.push_back(token);
            token.clear();
        } else {
            token += c;
        }
    }
    tokens.push_back(token);
    return tokens;
}

// Command handler
class CommandHandler {
private:
    FileExplorer& explorer;
    bool running;
    
    void handleCd(const vector<string>& args) {
        if (args.size() < 2) {
            cout << "Error: cd command requires a directory name" << endl;
            return;
        }
        
        // Join all arguments after 'cd' to handle spaces in directory names
        string target = args[1];
        for (size_t i = 2; i < args.size(); i++) {
            target += " " + args[i];
        }
        
        if (!explorer.navigate(target)) {
            cout << "Error: '" << target << "' is not a valid directory" << endl;
        }
    }
    
    void handleView(const vector<string>& args) {
        if (args.size() < 2) {
            cout << "Error: view command requires a file name" << endl;
            return;
        }
        
        // Join all arguments to handle spaces in file names
        string fileName = args[1];
        for (size_t i = 2; i < args.size(); i++) {
            fileName += " " + args[i];
        }
        
        if (!explorer.viewFile(fileName)) {
            cout << "Error: Could not view file '" << fileName << "'" << endl;
        }
    }
    
    void handleDelete(const vector<string>& args) {
        if (args.size() < 2) {
            cout << "Error: delete command requires an item name" << endl;
            return;
        }
        
        // Join all arguments to handle spaces in names
        string itemName = args[1];
        for (size_t i = 2; i < args.size(); i++) {
            itemName += " " + args[i];
        }
        
        explorer.deleteItem(itemName);
    }
    
    void handleEdit(const vector<string>& args) {
        if (args.size() < 2) {
            cout << "Error: edit command requires a file name" << endl;
            return;
        }
        
        // Join all arguments to handle spaces in file names
        string fileName = args[1];
        for (size_t i = 2; i < args.size(); i++) {
            fileName += " " + args[i];
        }
        
        if (!explorer.editFile(fileName)) {
            cout << "Error: Could not edit file '" << fileName << "'" << endl;
        }
    }
    
    void handleCopy(const vector<string>& args) {
        if (args.size() < 2) {
            cout << "Error: copy command requires an item name" << endl;
            return;
        }
        
        // Join all arguments to handle spaces in names
        string itemName = args[1];
        for (size_t i = 2; i < args.size(); i++) {
            itemName += " " + args[i];
        }
        
        if (!explorer.copyItem(itemName)) {
            cout << "Error: Could not copy '" << itemName << "'" << endl;
        }
    }
    
    void handleCut(const vector<string>& args) {
        if (args.size() < 2) {
            cout << "Error: cut command requires an item name" << endl;
            return;
        }
        
        // Join all arguments to handle spaces in names
        string itemName = args[1];
        for (size_t i = 2; i < args.size(); i++) {
            itemName += " " + args[i];
        }
        
        if (!explorer.cutItem(itemName)) {
            cout << "Error: Could not cut '" << itemName << "'" << endl;
        }
    }
    
    void handlePaste(const vector<string>& args) {
        if (!explorer.pasteItem()) {
            cout << "Error: Could not paste item" << endl;
        }
    }
    
    void handleMkdir(const vector<string>& args) {
        if (args.size() < 2) {
            cout << "Error: mkdir command requires a directory name" << endl;
            return;
        }
        
        // Join all arguments to handle spaces in directory names
        string dirName = args[1];
        for (size_t i = 2; i < args.size(); i++) {
            dirName += " " + args[i];
        }
        
        explorer.createDirectory(dirName);
    }
    
    void handleTouch(const vector<string>& args) {
        if (args.size() < 2) {
            cout << "Error: touch command requires a file name" << endl;
            return;
        }
        
        // Join all arguments to handle spaces in file names
        string fileName = args[1];
        for (size_t i = 2; i < args.size(); i++) {
            fileName += " " + args[i];
        }
        
        explorer.createFile(fileName);
    }
    
    void handleExit(const vector<string>& args) {
        running = false;
        cout << "Exiting file explorer..." << endl;
    }
    
    void handleHelp(const vector<string>& args) {
        if (args.size() > 1) {
            string command = args[1];
            if (command == "cd") {
                cout << "cd <directory_name> - Navigate to a directory\n";
                cout << "cd .. - Navigate to parent directory\n";
                cout << "cd ~ - Navigate to home directory\n";
            } else if (command == "view") {
                cout << "view <file_name> - Open file with system application\n";
            } else if (command == "delete") {
                cout << "delete <name> - Delete a file or directory\n";
            } else if (command == "edit") {
                cout << "edit <file_name> - Open file with system application\n";
            } else if (command == "copy") {
                cout << "copy <name> - Copy a file or directory\n";
            } else if (command == "cut") {
                cout << "cut <name> - Cut a file or directory\n";
            } else if (command == "paste") {
                cout << "paste - Paste copied item into current directory\n";
            } else if (command == "mkdir") {
                cout << "mkdir <name> - Create a new directory\n";
            } else if (command == "touch") {
                cout << "touch <name> - Create a new file (adds .txt if no extension)\n";
            } else if (command == "exit") {
                cout << "exit - Exit the file explorer\n";
            } else if (command == "help") {
                cout << "help - Display available commands\n";
                cout << "help <command> - Display detailed help for a command\n";
            } else if (command == "clear") {
                cout << "clear - Clear the console screen\n";
            } else if (command == "ls") {
                cout << "ls [options] - List files and folders (Linux-style)\n";
                cout << "  ls      - Basic listing\n";
                cout << "  ls -a   - Show all files (including hidden)\n";
                cout << "  ls -l   - Long format (permissions, size, date)\n";
                cout << "  ls -la  - All files with details\n";
                cout << "  ls -d   - Directories only\n";
                cout << "  ls -R   - Recursive listing\n";
            } else if (command == "dir") {
                cout << "dir [options] - List files and folders (Windows-style)\n";
                cout << "  dir      - Basic listing\n";
                cout << "  dir /a   - Show all files (including hidden)\n";
                cout << "  dir /ad  - Show directories only\n";
                cout << "  dir /s   - Recursive (subdirectories)\n";
                cout << "  dir /q   - Detailed list (permissions, size, date)\n";
                cout << "Can combine with ls flags: dir -la, dir /a /q\n";
            } else {
                cout << "No help available for '" << command << "'\n";
            }
        } else {
            cout << "\n╔═══════════════════════════════════════════════════════════════════╗\n";
            cout << "║                        Available Commands                         ║\n";
            cout << "╠═══════════════════════════════════════════════════════════════════╣\n";
            cout << "║ ls/dir [options]  - List directory contents                       ║\n";
            cout << "║ cd <directory>    - Navigate to directory                         ║\n";
            cout << "║ view <file>       - Open file with system app                     ║\n";
            cout << "║ edit <file>       - Edit file with system app                     ║\n";
            cout << "║ delete <name>     - Delete file or directory                      ║\n";
            cout << "║ copy <name>       - Copy file or directory                        ║\n";
            cout << "║ cut <name>        - Cut file or directory                         ║\n";
            cout << "║ paste             - Paste copied/cut item                         ║\n";
            cout << "║ mkdir <name>      - Create new directory                          ║\n";
            cout << "║ touch <name>      - Create new file                               ║\n";
            cout << "║ clear             - Clear screen                                  ║\n";
            cout << "║ exit              - Exit file explorer                            ║\n";
            cout << "║ help [command]    - Show help for specific command                ║\n";
            cout << "╚═══════════════════════════════════════════════════════════════════╝\n";
        }
    }
    
    void handleClear(const vector<string>& args) {
        clearScreen();
        drawBoxHeader("Console Based File Explorer");
    }
    
    void handleLs(const vector<string>& args) {
        bool showHidden = false;
        bool longFormat = false;
        bool dirsOnly = false;
        bool recursive = false;
        
        // Parse flags - support both Linux (-) and Windows (/) style
        for (size_t i = 1; i < args.size(); i++) {
            string arg = args[i];
            
            // Linux-style flags (e.g., -la, -al, -R)
            if (arg[0] == '-' && arg.length() > 1) {
                for (size_t j = 1; j < arg.length(); j++) {
                    switch (arg[j]) {
                        case 'a':
                            showHidden = true;
                            break;
                        case 'l':
                            longFormat = true;
                            break;
                        case 'd':
                            dirsOnly = true;
                            break;
                        case 'R':
                            recursive = true;
                            break;
                        default:
                            cout << "Unknown option: -" << arg[j] << endl;
                            return;
                    }
                }
            }
            // Windows-style flags (e.g., /a, /ad, /s, /q)
            else if (arg[0] == '/' && arg.length() > 1) {
                string flag = arg.substr(1);
                transform(flag.begin(), flag.end(), flag.begin(), ::tolower);
                
                if (flag == "a") {
                    showHidden = true;
                } else if (flag == "ad") {
                    dirsOnly = true;
                } else if (flag == "s") {
                    recursive = true;
                } else if (flag == "q") {
                    longFormat = true;
                } else {
                    cout << "Unknown option: /" << arg.substr(1) << endl;
                    return;
                }
            }
        }
        
        explorer.listDirectory(showHidden, longFormat, dirsOnly, recursive);
    }
    
public:
    CommandHandler(FileExplorer& explorer) : explorer(explorer), running(true) {}
    
    void processCommand(const string& commandLine) {
        vector<string> args = splitString(commandLine, ' ');
        if (args.empty()) {
            return;
        }
        
        string command = args[0];
        if (command == "cd") {
            handleCd(args);
        } else if (command == "view") {
            handleView(args);
        } else if (command == "delete" || command == "del" || command == "rm") {
            handleDelete(args);
        } else if (command == "edit") {
            handleEdit(args);
        } else if (command == "copy" || command == "cp") {
            handleCopy(args);
        } else if (command == "cut") {
            handleCut(args);
        } else if (command == "paste") {
            handlePaste(args);
        } else if (command == "mkdir") {
            handleMkdir(args);
        } else if (command == "touch") {
            handleTouch(args);
        } else if (command == "exit" || command == "quit") {
            handleExit(args);
        } else if (command == "help") {
            handleHelp(args);
        } else if (command == "clear" || command == "cls") {
            handleClear(args);
        } else if (command == "ls" || command == "dir") {
            handleLs(args);
        } else {
            cout << "Unknown command: " << command << ". Type 'help' for available commands." << endl;
        }
    }
    
    bool isRunning() const { return running; }
    
    void run() {
        string command;
        
        while (running) {
            setConsoleColor(COLOR_YELLOW);
            cout << explorer.getCurrentPath() << "> ";
            setConsoleColor(COLOR_RED);
            
            getline(cin, command);
            
            setConsoleColor(COLOR_RESET);
            
            if (!command.empty()) {
                processCommand(command);
            }
        }
    }
};

int main() {
    setupConsole();
    clearScreen();
    
    drawBoxHeader("Console Based File Explorer");
    
    FileExplorer explorer;
    
    CommandHandler handler(explorer);
    handler.run();
    setConsoleColor(COLOR_RESET);
    return 0;
}