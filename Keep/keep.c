#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_PATH_LENGTH 256
#define MAX_ENTRIES 100

typedef struct {
    char path[MAX_PATH_LENGTH];
    time_t last_modified;
} Entry;

typedef struct {
    Entry entries[MAX_ENTRIES];
    int count;
} TrackingFiles;

void init() {
    struct stat fileStat;
    if (stat(".keep", &fileStat) == 0 && S_ISDIR(fileStat.st_mode)) {
        printf(".keep already exists.\n");
    } else {
        if (mkdir(".keep", 0700) != 0) {
            printf("Failed to create .keep directory.\n");
        } else {
            FILE* tracking_files = fopen(".keep/tracking-files", "w");
            if (tracking_files == NULL) {
                printf("Failed to create tracking-files file.\n");
            }
            fclose(tracking_files);

            FILE* latest_version = fopen(".keep/latest-version", "w");
            if (latest_version == NULL) {
                printf("Failed to create latest-version file.\n");
            } else {
                fprintf(latest_version, "0");
                fclose(latest_version);
            }

            printf("Initialized .keep.\n");
        }
    }
}

// already track && update or delete
int update(const char* path, int check) {
    FILE *fp_input, *fp_output;
    fp_input = fopen(".keep/tracking-files", "r"); // Open file for reading
    fp_output = fopen(".keep/tracking-files.tmp", "w"); // Open temporary file for writing
    struct stat st;
    int result = 0, find = 0;

    if (fp_input == NULL || fp_output == NULL) {
        printf("Failed to open files for reading or writing.\n");
        exit(0);
    }

    char line[MAX_PATH_LENGTH];
    while (fgets(line, sizeof(line), fp_input)) {
        Entry entry;
        int num_fields = sscanf(line, "%s %ld", entry.path, &entry.last_modified);
        
        if (num_fields != 2) {
            printf("Error parsing line: %s\n", line);
            continue;
        }
        
        if (strcmp(entry.path, path) == 0) {
            if (stat(path, &st) == 0) {
                printf("일치하는 파일명 중 수정할 것 찾았다!\n");
                // Update the last_mod value
                entry.last_modified = st.st_mtime;
                result = 1, find = 1;
            }
        
        }
        if(find == 1 && check == 0) {
            find = 0;
            continue;
        }

        // Write the entry to the temporary file
        fprintf(fp_output, "%s %ld\n", entry.path, entry.last_modified);
    }

    if (rename(".keep/tracking-files.tmp", ".keep/tracking-files") != 0) {
        printf("Failed to update tracking-files.\n");
        exit(0);
    }

    fclose(fp_input);
    fclose(fp_output);

    // printf("어떻게 나오지!!!! %d\n", result);
    return result;
}

// track
void addFile(const char* path) {
    // 이미 동명의 파일의 add 되어 있는 경우
    if (update(path, 1)) {
        printf("File '%s' 수정\n", path);
        return;
    } else {
        struct stat st;
        if (stat(path, &st) != 0) {
            printf("Failed to get file information for '%s'.\n", path);
            return;
        } else {
            FILE *fp_output;
            fp_output = fopen(".keep/tracking-files", "a"); 			// Write File


             printf("path -> %s\n", path);
             printf("last_mod -> %ld\n", st.st_mtime);

             if (fp_output == NULL) {
                printf("Failed to open tracking-files file for writing.\n");
                return;
            }
            printf("새로운 파일 '%s'이 추가되었습니다! \n", path);
            fprintf(fp_output, "%s %ld\n", path, st.st_mtime); // Write path and last_mod values
            fclose(fp_output);

        }
    }
}

// untrack
void removeFile(const char* path) {
    int result = update(path, 0);
    if(result) 
        printf("File '%s' Delete Success\n", path);
    else
        printf("File '%s' Delete Failed\n", path);
}

void printVersions() {
    DIR* dir = opendir(".keep");
    if (dir == NULL) {
        printf("No versions found.\n");
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            printf("%s ", entry->d_name);
            char version_dir[MAX_PATH_LENGTH];
            snprintf(version_dir, sizeof(version_dir), ".keep/%s", entry->d_name);
            char note_file[MAX_PATH_LENGTH];
            snprintf(note_file, sizeof(note_file), "%s/note", version_dir);
            FILE* note = fopen(note_file, "r");
            if (note != NULL) {
                char note_content[256];
                fgets(note_content, sizeof(note_content), note);
                fclose(note);
                printf("%s", note_content);
            }
        }
    }
    closedir(dir);
}

// void store(const char* note, TrackingFiles* files) {
//     int updated = 0;
//     for (int i = 0; i < files->count; i++) {
//         struct stat st;
//         if (stat(files->entries[i].path, &st) != 0) {
//             printf("Failed to get file information for '%s'.\n", files->entries[i].path);
//             continue;
//         }

//         if (st.st_mtime > files->entries[i].last_modified) {
//             updated = 1;
//             break;
//         }
//     }

//     if (!updated) {
//         printf("Nothing to update.\n");
//         return;
//     }

//     DIR* keep_dir = opendir(".keep");
//     if (keep_dir == NULL) {
//         printf("Failed to open '.keep' directory.\n");
//         return;
//     }

//     struct dirent* entry;
//     int latest_version = 0;
//     while ((entry = readdir(keep_dir)) != NULL) {
//         if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
//             int version = atoi(entry->d_name);
//             if (version > latest_version) {
//                 latest_version = version;
//             }
//         }
//     }
//     closedir(keep_dir);

//     char version_dir[MAX_PATH_LENGTH];
//     snprintf(version_dir, sizeof(version_dir), ".keep/%d", latest_version + 1);
//     if (mkdir(version_dir, 0700) != 0) {
//         printf("Failed to create version directory.\n");
//         return;
//     }

//     char target_dir[MAX_PATH_LENGTH];
//     snprintf(target_dir, sizeof(target_dir), "%s/target", version_dir);
//     if (mkdir(target_dir, 0700) != 0) {
//         printf("Failed to create target directory.\n");
//         return;
//     }

//     for (int i = 0; i < files->count; i++) {
//         char target_path[MAX_PATH_LENGTH];
//         snprintf(target_path, sizeof(target_path), "%s/%s", target_dir, files->entries[i].path);

//         struct stat st;
//         if (stat(files->entries[i].path, &st) != 0) {
//             printf("Failed to get file information for '%s'.\n", files->entries[i].path);
//             continue;
//         }

//         if (st.st_mtime > files->entries[i].last_modified) {
//             if (access(files->entries[i].path, F_OK) == 0) {
//                 if (link(files->entries[i].path, target_path) != 0) {
//                     printf("Failed to create a hard link for '%s'.\n", files->entries[i].path);
//                     continue;
//                 }
//             } else {
//                 printf("File '%s' does not exist.\n", files->entries[i].path);
//                 continue;
//             }
//         } else {
//             char latest_version_path[MAX_PATH_LENGTH];
//             snprintf(latest_version_path, sizeof(latest_version_path), ".keep/%d/target/%s", latest_version, files->entries[i].path);
//             if (link(latest_version_path, target_path) != 0) {
//                 printf("Failed to create a hard link for '%s'.\n", files->entries[i].path);
//                 continue;
//             }
//         }
//     }

//     FILE* note_file = fopen(".keep/latest-version", "w");
//     if (note_file == NULL) {
//         printf("Failed to open 'latest-version' file.\n");
//         return;
//     }
//     fprintf(note_file, "%d", latest_version + 1);
//     fclose(note_file);

//     char note_file_path[MAX_PATH_LENGTH];
//     snprintf(note_file_path, sizeof(note_file_path), "%s/note", version_dir);
//     FILE* note = fopen(note_file_path, "w");
//     if (note == NULL) {
//         printf("Failed to open 'note' file.\n");
//         return;
//     }
//     fprintf(note, "%s", note);
//     fclose(note);

//     printf("Stored as version %d.\n", latest_version + 1);
// }

// void restore(int version, TrackingFiles* files) {
//     DIR* keep_dir = opendir(".keep");
//     if (keep_dir == NULL) {
//         printf("Failed to open '.keep' directory.\n");
//         return;
//     }

//     struct dirent* entry;
//     int latest_version = 0;
//     while ((entry = readdir(keep_dir)) != NULL) {
//         if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
//             int version_num = atoi(entry->d_name);
//             if (version_num > latest_version) {
//                 latest_version = version_num;
//             }
//         }
//     }
//     closedir(keep_dir);

//     if (version > latest_version || version < 1) {
//         printf("Invalid version number.\n");
//         return;
//     }

//     for (int i = 0; i < files->count; i++) {
//         struct stat st;
//         if (stat(files->entries[i].path, &st) != 0) {
//             printf("Failed to get file information for '%s'.\n", files->entries[i].path);
//             continue;
//         }

//         if (st.st_mtime > files->entries[i].last_modified) {
//             printf("File '%s' has been modified. Restore aborted.\n", files->entries[i].path);
//             return;
//         }
//     }

//     char version_dir[MAX_PATH_LENGTH];
//     snprintf(version_dir, sizeof(version_dir), ".keep/%d", version);

//     char target_dir[MAX_PATH_LENGTH];
//     snprintf(target_dir, sizeof(target_dir), "%s/target", version_dir);

//     DIR* version_dir_ptr = opendir(version_dir);
//     if (version_dir_ptr == NULL) {
//         printf("Failed to open version directory.\n");
//         return;
//     }

//     struct dirent* entry;
//     while ((entry = readdir(version_dir_ptr)) != NULL) {
//         if (entry->d_type == DT_REG) {
//             char source_path[MAX_PATH_LENGTH];
//             snprintf(source_path, sizeof(source_path), "%s/%s", target_dir, entry->d_name);
//             char dest_path[MAX_PATH_LENGTH];
//             snprintf(dest_path, sizeof(dest_path), "%s/%s", ".", entry->d_name);
//             if (rename(source_path, dest_path) != 0) {
//                 printf("Failed to restore file '%s'.\n", entry->d_name);
//             }
//         }
//     }
//     closedir(version_dir_ptr);

//     FILE* tracking_file = fopen(".keep/tracking-files", "w");
//     if (tracking_file == NULL) {
//         printf("Failed to open 'tracking-files' file.\n");
//         return;
//     }
//     for (int i = 0; i < files->count; i++) {
//         fprintf(tracking_file, "%s %ld\n", files->entries[i].path, files->entries[i].last_modified);
//     }
//     fclose(tracking_file);

//     printf("Restored version %d.\n", version);
// }

int main(int argc, char ** argv) {
    TrackingFiles file;
    file.count = 0;

    char command[256];
    char arg[256];

    if (argc >= 2) {
        strncpy(command, argv[1], MAX_PATH_LENGTH - 1);
        command[MAX_PATH_LENGTH - 1] = '\0';

        if (argc >= 3) {
            strncpy(arg, argv[2], MAX_PATH_LENGTH - 1);
            arg[MAX_PATH_LENGTH - 1] = '\0';
        }
    }
    if (strcmp(command, "init") == 0) {
        init();
    } else if (strcmp(command, "track") == 0) {
        printf("track!! -> %d\n", file.count);

        // keep track <file or directory>
        addFile(arg);
    } else if (strcmp(command, "untrack") == 0) {
        // keep untrack <file or directory>
        removeFile(arg);
    } else if (strcmp(command, "versions") == 0) {
        // keep versions
        printVersions();
    } 
    // else if (strcmp(command, "store") == 0) {
    //     // keep store "note"
    //     store(arg, &files);
    // } else if (strcmp(command, "restore") == 0) {
    //     // keep restore <version>
    //     int version = atoi(arg);
    //     restore(version, &files);
    // } 
    else {
        printf("Invalid command.\n");
    }

    return 0;
}
