# File-Explorer-OOP-project-2025
A Virtual File Explorer lets users manage files and folders through a console-based interface like cygwin.

# 🗂️ Virtual File Explorer

A console-based Virtual File Explorer built using Object-Oriented Programming (OOP) principles in C++. This project simulates basic file system operations with support for folders, `.txt`, and `.cpp` files. Designed using clean architecture and proper class relationships such as inheritance, composition, aggregation, and association.

# Features

### 1. Supported File Types
The application supports the following:
- 📁 Folders
- 📄 `.txt` files
- 💻 `.cpp` files
---
### 2. Directory Display
- Displays the **current directory** in full Windows-style path format (e.g., `root\Users\Documents\MyProject`)
- After each operation, the path and a list of items in the current directory are shown

**Example Output:**
root\Users\User\source
- files and folders are:
1. 📁 Documents
2. 📁 Downloads
3. 📁 Pictures
4. 📁 Desktop
5. 📄 Myfile.txt

---

### 3. Navigation
- `cd <folder_name>` – Navigate into a folder
- `cd ..` – Move to the parent directory

---

### 4. Create
- `mkdir <folder_name>` – Create a new folder
- `touch <file_name>` – Create a new `.txt` or `.cpp` file

---

### 5. Delete
- `delete <folder_name>` – Delete a folder
- `delete <file_name>` – Delete a file

---

### 6. Copy and Paste
- `copy <file_or_folder>` – Copy a file or an entire folder
- `paste` – Paste the last copied file or folder (deep copy supported)

---

### 7. View File Content
- `cd <file_name>` or `view <file_name>` – Display the content of `.txt` or `.cpp` files in the console

---

### 8. Edit File Content
- `edit <file_name>` – Edit the content of `.txt` or `.cpp` files
- Use:
  - `:w` – Save changes
  - `:q` – Quit editor
  - `:q!` – Quit without saving

---

### 9. Help Command
- `help` – Display a list of all commands
- `help <command_name>` – Show help for a specific command

---

### 10. Save on Exit
- On exiting the application:
  - Directory structure is saved to `hierarchy.txt` with proper indentation
  - All `.txt` and `.cpp` file contents are written to their respective files

---
## UML Diagram
![UML](https://github.com/Toheed-Ali/File-Explorer-OOP-project-2025/blob/main/File%20Explorer%20UML%20Diagram.png)
---
## Flowchart/Activity Diagram
![Flowchart](https://github.com/Toheed-Ali/File-Explorer-OOP-project-2025/blob/main/Flowchart-Activity%20Diagram.png)
---
## Class Relationships

### 1. Inheritance
- `File` → `FileSystemObject`
- `Directory` → `FileSystemObject`

### 2. Composition (Strong Ownership)
- `Directory` ◆ contains multiple `FileSystemObject` instances
- When a `Directory` is destroyed, all its contents are destroyed

### 3. Aggregation (Weak Ownership)
- `FileExplorer` ◇→ `Directory`  
  (*has* a `currentDirectory` but does not own the root exclusively)
- `CommandHandler` ◇→ `FileExplorer`  
  (*uses* `FileExplorer` without managing its lifetime)

### 4. Association (Usage Dependency)
- `FileEditor` ── `File`  
  (*temporarily edits a file*)
- `CommandHandler` ── `FileEditor`  
  (*creates a `FileEditor` when needed*)

---

## 🛠️ Technologies Used
- C++
- Object-Oriented Design
- File Handling
- Custom Console Interface

---

## 📌 Note
This application is a simulation and does not interact with the real file system. All operations are virtual and exist within the program's data structures until exit, when data is written to disk.
<img src="https://t.bkit.co/w_6835fc6c44aca.gif" />
