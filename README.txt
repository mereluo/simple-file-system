File System Implementation
=========================

Team Members
-----------
Meredith Luo, Jie Ji

Compilation Instructions
----------------------
1. To compile the program, use the following commands:
   ```
   make clean    # Clean up any previous build
   make         # Compile the program
   ```

Execution Instructions
--------------------
1. The program can be executed using the following command format:
   ```
   ./filesys -s <test_file>
   ```

2. Example command:
   ```
   ./filesys -s test_disk_full.txt
   ```

Important Notes
-------------
1. Before running each test file, it's crucial to:
   a. First clean up resources with: make clean
   b. Recompile with: make
   c. Then run the test file

2. This ensures each test starts with a fresh state and prevents any interference from previous test runs.

Example Test Sequence
-------------------
To run multiple test files:
```
make clean
make
./filesys -s test_basic.txt

make clean
make
./filesys -s test_file_size.txt

make clean
make
./filesys -s test_dir_full.txt

make clean
make
./filesys -s test_disk_full.txt
```

Implementation Details
--------------------
The file system implementation supports the following commands:
- mkdir: Create a new directory
- ls: List contents of current directory
- cd: Change current directory
- home: Return to root directory
- rmdir: Remove a directory
- create: Create a new file
- append: Append data to a file
- cat: Display contents of a file
- tail: Display last 10 lines of a file
- rm: Remove a file
- stat: Display file/directory information

All features have been implemented as per the specifications.

Known Issues
-----------
No known issues. All features are working as expected.

File Structure
-------------
The implementation is contained in two main files:
1. FileSys.h - Header file containing class definitions
2. FileSys.cpp - Implementation file containing all the file system functionality 