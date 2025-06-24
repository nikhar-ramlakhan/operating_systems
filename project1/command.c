#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void listDir() {
    DIR *dir = opendir(".");
     // char *error_opening_dir = "Error: Unable to open current directory.\n";
    if (dir == NULL) {
        write(1, "Error: Unable to open current directory.\n", strlen("Error: Unable to open current directory.\n"));
        return;
    }

    struct dirent *dirEntry;
    char buffer[4096];
    size_t offset = 0;

    while ((dirEntry = readdir(dir)) != NULL) {
        if (dirEntry->d_type == DT_REG || dirEntry->d_type == DT_DIR) {
            size_t len = strlen(dirEntry->d_name);
            memcpy(buffer + offset, dirEntry->d_name, len);
            offset += len;
            buffer[offset++] = ' ';
        }
    }
    closedir(dir);
    write(1, buffer, offset);
    write(1, "\n", 1);
}

void showCurrentDir() {
    char cwd[4096];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        // char *error_getting_cwd = "Error: Unable to retrieve current directory.\n";
        write(1, "Error: Unable to retrieve current directory.\n", strlen("Error: Unable to retrieve current directory.\n"));
        return;
    }
    write(1, cwd, strlen(cwd));
    write(1, "\n", 1);
}

void changeDir(char *dirName) {
    if (chdir(dirName) == -1) {
        char *error_changing_dir = "Error: Failed to change directory.\n";
        write(1, error_changing_dir, strlen(error_changing_dir));
    }
}

void makeDir(char *dirName) {
    if (mkdir(dirName, 0777) == -1) {
        char *error_creating_dir = "Error: Failed to create directory or directory already exists.\n";
        write(1, error_creating_dir, strlen(error_creating_dir));
    }
}

void copyFile(char *srcPath, char *destPath) {
    int srcFile = -1, destFile = -1;
    ssize_t bytes_read;
    char buffer[256];
    struct stat stats;

    // Attempt to open the source file first to ensure it exists
    srcFile = open(srcPath, O_RDONLY);
    if (srcFile == -1) {
        const char *err_open_src = "Error: Could not open source file.\n";
        write(1, err_open_src, strlen(err_open_src));
        return;
    }

    // Check if the destination is a directory
    if (stat(destPath, &stats) == 0 && S_ISDIR(stats.st_mode)) {
        char new_path[512];
        char *filename = strrchr(srcPath, '/');
        if (filename == NULL) {
            filename = srcPath;  // No '/' found, srcPath is the filename.
        } else {
            filename++;  // Increment to skip the '/' character.
        }
        
        snprintf(new_path, sizeof(new_path), "%s/%s", destPath, filename);
        destFile = open(new_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    } else {
        // Open the destination file as it is not a directory
        destFile = open(destPath, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    }

    if (destFile == -1) {
        const char *err_open_dest = "Error: Could not open or create destination file.\n";
        write(1, err_open_dest, strlen(err_open_dest));
        close(srcFile);  // Close the source file descriptor to prevent a leak
        return;
    }

    // Copy content from source to destination
    while ((bytes_read = read(srcFile, buffer, sizeof(buffer))) > 0) {
        if (write(destFile, buffer, bytes_read) != bytes_read) {
            const char *err_write_full = "Error: Could not write all data to destination.\n";
            write(1, err_write_full, strlen(err_write_full));
            break;
        }
    }

    if (bytes_read == -1) {
        const char *err_read_src = "Error: Failed to read from source file.\n";
        write(1, err_read_src, strlen(err_read_src));
    }

    // Close both file descriptors
    close(srcFile);
    close(destFile);
}


void moveFile(char *sourcePath, char *destinationPath) {
    int srcFile, destFile;
    ssize_t bytesRead;
    char buffer[256];

    struct stat stats;
    if (stat(destinationPath, &stats) == 0 && S_ISDIR(stats.st_mode)) {
        char *filename = strrchr(sourcePath, '/');
        if (filename == NULL) {
            filename = sourcePath;  // No '/' found, the entire sourcePath is the filename.
        } else {
            filename++;  // Skip the '/' character.
        }
        char newPath[512];
        snprintf(newPath, sizeof(newPath), "%s/%s", destinationPath, filename);
        destFile = open(newPath, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

        if (destFile == -1) {
            const char *error_create = "Error: Failed to create file in directory.\n";
            write(1, error_create, strlen(error_create));
            return;
        }
    } else {
        srcFile = open(sourcePath, O_RDONLY);
        if (srcFile == -1) {
            const char *error_source = "Error: Could not open source file.\n";
            write(1, error_source, strlen(error_source));
            return;
        }

        destFile = open(destinationPath, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (destFile == -1) {
            const char *error_dest = "Error: Could not open destination file.\n";
            write(1, error_dest, strlen(error_dest));
            close(srcFile);
            return;
        }

        while ((bytesRead = read(srcFile, buffer, sizeof(buffer))) > 0) {
            if (write(destFile, buffer, bytesRead) != bytesRead) {
                const char *error_write = "Error: Could not write to destination file.\n";
                write(1, error_write, strlen(error_write));
                break;
            }
        }

        if (bytesRead == -1) {
            const char *error_read = "Error: Could not read from source file.\n";
            write(1, error_read, strlen(error_read));
        }

        close(srcFile);
        close(destFile);
    }

    if (unlink(sourcePath) == -1) {
        const char *error_unlink = "Error: Failed to remove source file.\n";
        write(1, error_unlink, strlen(error_unlink));
    }
}

void deleteFile(char *filename) {
    if (unlink(filename) == -1) {
        const char *unlink_error = "Error: Failed to delete file.\n";
        write(1, unlink_error, strlen(unlink_error));
    } else {
        const char *unlink_message = "File successfully deleted.\n";
        write(1, unlink_message, strlen(unlink_message));
    }
}

void displayFile(char *filename) {
    int fileFd = open(filename, O_RDONLY);
    if (fileFd == -1) {
        const char *error_open = "Error: Could not open file.\n";
        write(1, error_open, strlen(error_open));
        return;
    }

    char buffer[1024];
    ssize_t bytesRead;
    while ((bytesRead = read(fileFd, buffer, sizeof(buffer))) > 0) {
        if (write(1, buffer, bytesRead) != bytesRead) {
            const char *error_write = "Error: Could not write to output.\n";
            write(1, error_write, strlen(error_write));
            break;
        }
    }

    if (bytesRead == -1) {
        const char *error_read = "Error: Could not read file.\n";
        write(1, error_read, strlen(error_read));
    }

    close(fileFd);
}