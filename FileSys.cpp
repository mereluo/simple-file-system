// Computing Systems: File System
// Implements the file system commands that are available to the shell.

#include <cstring>
#include <iostream>
#include <algorithm>
using namespace std;

#include "FileSys.h"
#include "BasicFileSys.h"
#include "Blocks.h"

// mounts the file system
void FileSys::mount() {
  bfs.mount();
  curr_dir = 1;
}

// unmounts the file system
void FileSys::unmount() {
  bfs.unmount();
}

// Helper function to check if a block is a directory
bool FileSys::is_directory(short block_num) {
    dirblock_t dirblock;
    bfs.read_block(block_num, (void *)&dirblock);
    return dirblock.magic == DIR_MAGIC_NUM;
}

// make a directory
void FileSys::mkdir(const char *name)
{
    // Check if filename is too long
    if (strlen(name) > MAX_FNAME_SIZE) {
        cout << "File name is too long" << endl;
        return;
    }

    // Read current directory
    dirblock_t curr_dir_block;
    bfs.read_block(curr_dir, (void *)&curr_dir_block);

    // Check if directory is full
    if (curr_dir_block.num_entries >= MAX_DIR_ENTRIES) {
        cout << "Directory is full" << endl;
        return;
    }

    // Check if directory already exists
    for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        if (curr_dir_block.dir_entries[i].block_num != 0 && 
            strcmp(curr_dir_block.dir_entries[i].name, name) == 0 &&
            is_directory(curr_dir_block.dir_entries[i].block_num)) {
            cout << "Directory exists" << endl;
            return;
        }
    }

    // Get a free block for the new directory
    short new_dir_block = bfs.get_free_block();
    if (new_dir_block == 0) {
        cout << "Disk is full" << endl;
        return;
    }

    // Initialize new directory block
    dirblock_t new_dir;
    new_dir.magic = DIR_MAGIC_NUM;
    new_dir.num_entries = 0;
    for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        new_dir.dir_entries[i].block_num = 0;
    }
    bfs.write_block(new_dir_block, (void *)&new_dir);

    // Add entry to current directory
    for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        if (curr_dir_block.dir_entries[i].block_num == 0) {
            strcpy(curr_dir_block.dir_entries[i].name, name);
            curr_dir_block.dir_entries[i].block_num = new_dir_block;
            curr_dir_block.num_entries++;
            break;
        }
    }

    // Write back current directory
    bfs.write_block(curr_dir, (void *)&curr_dir_block);
}

// switch to a directory
void FileSys::cd(const char *name)
{
    // Read current directory
    dirblock_t curr_dir_block;
    bfs.read_block(curr_dir, (void *)&curr_dir_block);

    // Find the directory entry
    for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        if (curr_dir_block.dir_entries[i].block_num != 0 && 
            strcmp(curr_dir_block.dir_entries[i].name, name) == 0) {
            
            // Check if it's a directory
            if (!is_directory(curr_dir_block.dir_entries[i].block_num)) {
                cout << "File is not a directory" << endl;
                return;
            }

            // Change to the new directory
            curr_dir = curr_dir_block.dir_entries[i].block_num;
            return;
        }
    }

    // If we get here, the directory wasn't found
    cout << "File does not exist" << endl;
}

// switch to home directory
void FileSys::home() {
    curr_dir = 1;  // Block 1 is always the home directory
}

// remove a directory
void FileSys::rmdir(const char *name)
{
    // Read current directory
    dirblock_t curr_dir_block;
    bfs.read_block(curr_dir, (void *)&curr_dir_block);

    // Find the directory entry
    for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        if (curr_dir_block.dir_entries[i].block_num != 0 && 
            strcmp(curr_dir_block.dir_entries[i].name, name) == 0) {
            
            // Check if it's a directory
            if (!is_directory(curr_dir_block.dir_entries[i].block_num)) {
                cout << "File is not a directory" << endl;
                return;
            }

            // Read the directory to be removed
            dirblock_t dir_to_remove;
            bfs.read_block(curr_dir_block.dir_entries[i].block_num, (void *)&dir_to_remove);

            // Check if directory is empty
            if (dir_to_remove.num_entries > 0) {
                cout << "Directory is not empty" << endl;
                return;
            }

            // Reclaim the directory block
            bfs.reclaim_block(curr_dir_block.dir_entries[i].block_num);

            // Remove the entry from current directory
            curr_dir_block.dir_entries[i].block_num = 0;
            curr_dir_block.num_entries--;
            bfs.write_block(curr_dir, (void *)&curr_dir_block);
            return;
        }
    }

    // If we get here, the directory wasn't found
    cout << "File does not exist" << endl;
}

// list the contents of current directory
void FileSys::ls()
{
    // Read current directory
    dirblock_t curr_dir_block;
    bfs.read_block(curr_dir, (void *)&curr_dir_block);

    bool found_entry = false;

    // Print each entry
    for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        if (curr_dir_block.dir_entries[i].block_num != 0) {
            cout << curr_dir_block.dir_entries[i].name;
            // Add '/' suffix for directories
            if (is_directory(curr_dir_block.dir_entries[i].block_num)) {
                cout << "/";
            }
            cout << endl;
            found_entry = true;
        }
    }

    // If no entries found, print a newline
    if (!found_entry) {
        cout << endl;
    }
}

// create an empty data file
void FileSys::create(const char *name)
{
    // Check if filename is too long
    if (strlen(name) > MAX_FNAME_SIZE) {
        cout << "File name is too long" << endl;
        return;
    }

    // Read current directory
    dirblock_t curr_dir_block;
    bfs.read_block(curr_dir, (void *)&curr_dir_block);

    // Check if directory is full
    if (curr_dir_block.num_entries >= MAX_DIR_ENTRIES) {
        cout << "Directory is full" << endl;
        return;
    }

    // Check if file already exists
    for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        if (curr_dir_block.dir_entries[i].block_num != 0 && 
            strcmp(curr_dir_block.dir_entries[i].name, name) == 0) {
            cout << "File exists" << endl;
            return;
        }
    }

    // Get a free block for the inode
    short inode_block = bfs.get_free_block();
    if (inode_block == 0) {
        cout << "Disk is full" << endl;
        return;
    }

    // Initialize inode
    inode_t inode;
    inode.magic = INODE_MAGIC_NUM;
    inode.size = 0;
    for (int i = 0; i < MAX_DATA_BLOCKS; i++) {
        inode.blocks[i] = 0;
    }
    bfs.write_block(inode_block, (void *)&inode);

    // Add entry to current directory
    for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        if (curr_dir_block.dir_entries[i].block_num == 0) {
            strcpy(curr_dir_block.dir_entries[i].name, name);
            curr_dir_block.dir_entries[i].block_num = inode_block;
            curr_dir_block.num_entries++;
            break;
        }
    }

    // Write back current directory
    bfs.write_block(curr_dir, (void *)&curr_dir_block);
}

// append data to a data file
void FileSys::append(const char *name, const char *data)
{
    // Read current directory
    dirblock_t curr_dir_block;
    bfs.read_block(curr_dir, (void *)&curr_dir_block);

    // Find the file entry
    short inode_block = 0;
    for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        if (curr_dir_block.dir_entries[i].block_num != 0 && 
            strcmp(curr_dir_block.dir_entries[i].name, name) == 0) {
            
            // Check if it's a directory
            if (is_directory(curr_dir_block.dir_entries[i].block_num)) {
                cout << "File is a directory" << endl;
                return;
            }

            inode_block = curr_dir_block.dir_entries[i].block_num;
            break;
        }
    }

    if (inode_block == 0) {
        cout << "File does not exist" << endl;
        return;
    }

    // Read the inode
    inode_t inode;
    bfs.read_block(inode_block, (void *)&inode);

    // Calculate data size
    int data_size = strlen(data);
    if (inode.size + data_size > MAX_FILE_SIZE) {
        cout << "Append exceeds maximum file size" << endl;
        return;
    }

    // Find the last block with data
    int last_block_index = -1;
    for (int i = 0; i < MAX_DATA_BLOCKS; i++) {
        if (inode.blocks[i] != 0) {
            last_block_index = i;
        }
    }

    // Calculate how much space is left in the last block
    int space_in_last_block = 0;
    if (last_block_index != -1) {
        space_in_last_block = BLOCK_SIZE - (inode.size % BLOCK_SIZE);
    }

    // Append data
    int data_index = 0;
    while (data_index < data_size) {
        // If we need a new block
        if (last_block_index == -1 || space_in_last_block == 0) {
            // Get a new block
            short new_block = bfs.get_free_block();
            if (new_block == 0) {
                cout << "Disk is full" << endl;
                return;
            }

            // Update inode
            last_block_index++;
            inode.blocks[last_block_index] = new_block;
            space_in_last_block = BLOCK_SIZE;
        }

        // Read the current block
        datablock_t block;
        bfs.read_block(inode.blocks[last_block_index], (void *)&block);

        // Calculate how much to write in this block
        int write_size = min(space_in_last_block, data_size - data_index);

        // Write the data
        for (int i = 0; i < write_size; i++) {
            block.data[BLOCK_SIZE - space_in_last_block + i] = data[data_index + i];
        }

        // Write back the block
        bfs.write_block(inode.blocks[last_block_index], (void *)&block);

        // Update counters
        data_index += write_size;
        space_in_last_block -= write_size;
        inode.size += write_size;
    }

    // Write back the inode
    bfs.write_block(inode_block, (void *)&inode);
}

// display the contents of a data file
void FileSys::cat(const char *name)
{
    // Read current directory
    dirblock_t curr_dir_block;
    bfs.read_block(curr_dir, (void *)&curr_dir_block);

    // Find the file entry
    short inode_block = 0;
    for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        if (curr_dir_block.dir_entries[i].block_num != 0 && 
            strcmp(curr_dir_block.dir_entries[i].name, name) == 0) {
            
            // Check if it's a directory
            if (is_directory(curr_dir_block.dir_entries[i].block_num)) {
                cout << "File is a directory" << endl;
                return;
            }

            inode_block = curr_dir_block.dir_entries[i].block_num;
            break;
        }
    }

    if (inode_block == 0) {
        cout << "File does not exist" << endl;
        return;
    }

    // Read the inode
    inode_t inode;
    bfs.read_block(inode_block, (void *)&inode);

    // Print the file contents
    int bytes_printed = 0;
    for (int i = 0; i < MAX_DATA_BLOCKS && bytes_printed < inode.size; i++) {
        if (inode.blocks[i] != 0) {
            datablock_t block;
            bfs.read_block(inode.blocks[i], (void *)&block);

            // Calculate how many bytes to print from this block
            int bytes_to_print = std::min((int)BLOCK_SIZE, (int)(inode.size - bytes_printed));
            
            // Print the bytes
            for (int j = 0; j < bytes_to_print; j++) {
                cout << block.data[j];
            }
            
            bytes_printed += bytes_to_print;
        }
    }
    cout << endl;
}

// display the last N bytes of the file
void FileSys::tail(const char *name, unsigned int n)
{
    // Read current directory
    dirblock_t curr_dir_block;
    bfs.read_block(curr_dir, (void *)&curr_dir_block);

    // Find the file entry
    short inode_block = 0;
    for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        if (curr_dir_block.dir_entries[i].block_num != 0 && 
            strcmp(curr_dir_block.dir_entries[i].name, name) == 0) {
            
            // Check if it's a directory
            if (is_directory(curr_dir_block.dir_entries[i].block_num)) {
                cout << "File is a directory" << endl;
                return;
            }

            inode_block = curr_dir_block.dir_entries[i].block_num;
            break;
        }
    }

    if (inode_block == 0) {
        cout << "File does not exist" << endl;
        return;
    }

    // Read the inode
    inode_t inode;
    bfs.read_block(inode_block, (void *)&inode);

    // If n >= file size, print the whole file
    if (n >= inode.size) {
        cat(name);
        return;
    }

    // Calculate starting position
    int start_pos = inode.size - n;
    int bytes_printed = 0;

    // Find the starting block
    int start_block = start_pos / BLOCK_SIZE;
    int start_offset = start_pos % BLOCK_SIZE;

    // Print the file contents
    for (int i = start_block; i < MAX_DATA_BLOCKS && bytes_printed < n; i++) {
        if (inode.blocks[i] != 0) {
            datablock_t block;
            bfs.read_block(inode.blocks[i], (void *)&block);

            // Calculate how many bytes to print from this block
            int bytes_to_print;
            if (i == start_block) {
                bytes_to_print = std::min((int)(BLOCK_SIZE - start_offset), (int)(n - bytes_printed));
                for (int j = start_offset; j < start_offset + bytes_to_print; j++) {
                    cout << block.data[j];
                }
            } else {
                bytes_to_print = std::min((int)BLOCK_SIZE, (int)(n - bytes_printed));
                for (int j = 0; j < bytes_to_print; j++) {
                    cout << block.data[j];
                }
            }
            
            bytes_printed += bytes_to_print;
        }
    }
    cout << endl;
}

// delete a data file
void FileSys::rm(const char *name)
{
    // Read current directory
    dirblock_t curr_dir_block;
    bfs.read_block(curr_dir, (void *)&curr_dir_block);

    // Find the file entry
    for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        if (curr_dir_block.dir_entries[i].block_num != 0 && 
            strcmp(curr_dir_block.dir_entries[i].name, name) == 0) {
            
            // Check if it's a directory
            if (is_directory(curr_dir_block.dir_entries[i].block_num)) {
                cout << "File is a directory" << endl;
                return;
            }

            // Read the inode
            inode_t inode;
            bfs.read_block(curr_dir_block.dir_entries[i].block_num, (void *)&inode);

            // Reclaim all data blocks
            for (int j = 0; j < MAX_DATA_BLOCKS; j++) {
                if (inode.blocks[j] != 0) {
                    bfs.reclaim_block(inode.blocks[j]);
                }
            }

            // Reclaim the inode block
            bfs.reclaim_block(curr_dir_block.dir_entries[i].block_num);

            // Remove the entry from current directory
            curr_dir_block.dir_entries[i].block_num = 0;
            curr_dir_block.num_entries--;
            bfs.write_block(curr_dir, (void *)&curr_dir_block);
            return;
        }
    }

    // If we get here, the file wasn't found
    cout << "File does not exist" << endl;
}

// display stats about file or directory
void FileSys::stat(const char *name)
{
    // Read current directory
    dirblock_t curr_dir_block;
    bfs.read_block(curr_dir, (void *)&curr_dir_block);

    // Find the file entry
    short block_num = 0;
    for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        if (curr_dir_block.dir_entries[i].block_num != 0 && 
            strcmp(curr_dir_block.dir_entries[i].name, name) == 0) {
            block_num = curr_dir_block.dir_entries[i].block_num;
            break;
        }
    }

    if (block_num == 0) {
        cout << "File does not exist" << endl;
        return;
    }

    // Check if it's a directory
    if (is_directory(block_num)) {
        cout << "Directory name: " << name << "/" << endl;
        cout << "Directory block: " << block_num << endl;
    } else {
        // Read the inode
        inode_t inode;
        bfs.read_block(block_num, (void *)&inode);

        // Count number of blocks used
        int num_blocks = 1; // Count the inode block
        for (int i = 0; i < MAX_DATA_BLOCKS; i++) {
            if (inode.blocks[i] != 0) {
                num_blocks++;
            }
        }

        cout << "Inode block: " << block_num << endl;
        cout << "Bytes in file: " << inode.size << endl;
        cout << "Number of blocks: " << num_blocks << endl;
        cout << "First block: " << (inode.blocks[0] != 0 ? inode.blocks[0] : 0) << endl;
    }
}
