#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h> // dirname 함수 사용을 위한 헤더 파일 추가


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


// 수정 함!!!
void add_entry(FILE *fp, const char *path, int last_modified) {
    fprintf(fp, "%s %d\n", path, last_modified);
}

// 디렉토리를 track 할 경우 하위 폴더 및 파일까지 저장할 때 사용되는 함수
void traverse_directory(const char *path, FILE *fp_output) {
    DIR *dir = opendir(path);
    if (dir == NULL) {
        printf("Failed to open directory: %s\n", path);
        return;
    }
    // dirent -> 디렉토리 엔트리를 나타내는 구조체
    struct dirent *d_entry;
    char file_path[MAX_PATH_LENGTH];
    // readdir를 통해 하위 폴더 및 파일 존재할 때까지 반복
    while ((d_entry = readdir(dir)) != NULL) {
        if (strcmp(d_entry->d_name, ".") == 0 || strcmp(d_entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(file_path, sizeof(file_path), "%s/%s", path, d_entry->d_name);

        struct stat st;
        if (stat(file_path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                // 디렉토리일 경우 재귀적으로 탐색
                traverse_directory(file_path, fp_output);
            } else {
                // 파일일 경우 추적 파일에 내용 추가
                add_entry(fp_output, file_path, 0);
            }
        }
    }

    closedir(dir);
}
// 수정 함!!!


// already track && update or delete
int update(const char* path, const char* readPath, int check) {
    FILE *fp_input, *fp_output;
    fp_input = fopen(readPath, "r"); // Open file for reading
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
        
        if(check == 0 || check == 1) {
            // track에서 보낸 path와 tracking-files 내부의 저장된 path 중 동일한 이름이 발견 됨
            if (strstr(entry.path, path) != NULL) {
                if (stat(entry.path, &st) == 0) {
                    printf("일치하는 파일명 중 수정할 것 찾았다!\n");
                    // Update the last_modified value
                    entry.last_modified = st.st_mtime;
                    result = 1, find = 1;
                }
            }
            if(find == 1 && check == 0) {
                find = 0;
                continue;
            }
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
    // 이미 동명의 파일의 add 되어 있는 경우, 1 -> add
    const char* readPath = ".keep/tracking-files";
    if (update(path, readPath, 1)) {
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
            // 새로 추가된 항목이 '디렉토리'
            if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) {
                // 디렉토리인 경우 하위 파일 및 디렉토리 순회하며 내용 추가
                traverse_directory(path, fp_output);
            // 새로 추가된 항목이 '파일'
            } else {
                // Write path and last_mod values   
                fprintf(fp_output, "%s %d\n", path, 0);
                fclose(fp_output);
            }
            printf("새로운 path '%s'이 추가되었습니다! \n", path);
        }
    }
}

// untrack
void removeFile(const char* path) {
    // 0 -> delete
    const char* readPath = ".keep/tracking-files";
    int result = update(path,readPath, 0);
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
            char ver_d[MAX_PATH_LENGTH];
            snprintf(ver_d, sizeof(ver_d), ".keep/%s", entry->d_name);
            char note_file[MAX_PATH_LENGTH];
            snprintf(note_file, sizeof(note_file), "%s/note", ver_d);
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
// traking-files의 내용을 담아 줌
void filled_files(TrackingFiles* files) {
    FILE *fp_input;
    fp_input = fopen(".keep/tracking-files", "r"); // Open file for reading
    struct stat st;
    int result = 0, find = 0;

    if (fp_input == NULL) {
        printf("Failed to open files for reading or writing.\n");
        exit(0);
    }

    char line[MAX_PATH_LENGTH];

    // 초기화
    files->count = 0;

    while (fgets(line, sizeof(line), fp_input)) {
        Entry entry;
        int num_fields = sscanf(line, "%s %ld", entry.path, &entry.last_modified);
        
        if (num_fields != 2) {
            printf("Error parsing line: %s\n", line);
            continue;
        } else {
            // 새로운 entry를 배열에 추가
            files->count++;
            files->entries[files->count - 1] = entry;
        }
    }
    fclose(fp_input);
}




// ------ 수정 중 ---------

void store(char* note_name) {
    TrackingFiles files;
    int updated = 0;
    // 원본 tracking-files에 있는 data를 files에 담아 옴
    filled_files(&files);

    printf("check count -> %d\n", files.count);
    printf("check entries count -> %s\n", files.entries[0].path);

    for (int i = 0; i < files.count; i++) {
        struct stat st;
        if (stat(files.entries[i].path, &st) != 0) {
            printf("Failed to get information of '%s'.\n", files.entries[i].path);
            continue;
        }

        if (st.st_mtime > files.entries[i].last_modified) {
            updated = 1;
            break;
        }
    }

    if (!updated) {
        printf("Nothing to store.\n");
        return;
    }

    DIR* keep_loc = opendir(".keep");
    if (keep_loc == NULL) {
        printf("'.keep' directory is not exist\n");
        return;
    }

    // check latest version store file name
    struct dirent* entry;
    int latest_ver = 0;
    while ((entry = readdir(keep_loc)) != NULL) {
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            int ver = atoi(entry->d_name);
            if (ver > latest_ver) {
                latest_ver = ver;
            }
        }
    }
    closedir(keep_loc);

    // create version directory
    char ver_d[MAX_PATH_LENGTH];
    snprintf(ver_d, sizeof(ver_d), ".keep/%d", latest_ver + 1);
    if (mkdir(ver_d, 0700) != 0) {
        printf("Failed to create version directory.\n");
        return;
    }
    // create tracking-files
    char tracking_d[MAX_PATH_LENGTH];
    snprintf(tracking_d, sizeof(tracking_d), "%s/tracking-files", ver_d);
    FILE* tracking_files = fopen(tracking_d, "w");
    if (tracking_files == NULL) {
        printf("Failed to create tracking-files.\n");
        return;
    }

    // create target directory
    char target_d[MAX_PATH_LENGTH];
    snprintf(target_d, sizeof(target_d), "%s/target", ver_d);
    if (mkdir(target_d, 0700) != 0) {
        printf("Failed to create target directory.\n");
        return;
    }


    for (int i = 0; i < files.count; i++) {
        char target_path[MAX_PATH_LENGTH];
        // snprintf(target_path, sizeof(target_path), "%s/%s", target_d, files.entries[i].path);

        struct stat st;
        if (stat(files.entries[i].path, &st) != 0) {
            printf("Failed to get file information for '%s'.\n", files.entries[i].path);
            continue;
        }

        if (st.st_mtime > files.entries[i].last_modified) {
            if (access(files.entries[i].path, F_OK) == 0) {
                 fprintf(tracking_files, "%s %ld\n", files.entries[i].path, st.st_mtime);
            } else {
                printf("File '%s' does not exist.\n", files.entries[i].path);
                continue;
            }
        } else {
            fprintf(tracking_files, "%s %ld\n", files.entries[i].path, files.entries[i].last_modified);
            char latest_ver_path[MAX_PATH_LENGTH];
            snprintf(latest_ver_path, sizeof(latest_ver_path), ".keep/%d/target/%s", latest_ver, files.entries[i].path);
        }
    }

    fclose(tracking_files);


    // store된 tracking-files의 내용을 .keep/tracking-files에 update
    const char* readPath = tracking_d;
    int result = update(0, readPath, 2);
    printf("store에서 ressult -> %d\n", result);


    FILE* latest_info = fopen(".keep/latest-version", "w");
    if (latest_info == NULL) {
        printf("'latest-version' file is not exist.\n");
        return;
    }
    fprintf(latest_info, "%d", latest_ver + 1);
    fclose(latest_info);

    char note_path[MAX_PATH_LENGTH];
    snprintf(note_path, sizeof(note_path), "%s/note", ver_d);

    FILE* note = fopen(note_path, "w");
    if (note == NULL) {
        printf("'note' file is not exist.\n");
        return;
    }
    fprintf(note, "%s", note_name);
    fclose(note);

    printf("Stored as version %d.\n", latest_ver + 1);
}

// -------- 수정 중 ---------



void restore(int version) {
    TrackingFiles files;
    // tracking-files에 있는 data를 files에 담아 옴
    filled_files(&files);

    DIR* keep_loc = opendir(".keep");
    if (keep_loc == NULL) {
        printf("'.keep' directory is not exist.\n");
        return;
    }

    struct dirent* entry;
    int latest_ver = 0;
    // keep의 모든 파일들을 읽을 때까지 반복
    while ((entry = readdir(keep_loc)) != NULL) {
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            int ver = atoi(entry->d_name);
            if (ver > latest_ver) {
                latest_ver = ver;
            }
        }
    }
    closedir(keep_loc);

    if (version > latest_ver || version < 1) {
        printf("Version not found.\n");
        return;
    }

    // latest-version 수정 내용이랑 해당 파일의 실제 수정 내용 비교
    for (int i = 0; i < files.count; i++) {
        struct stat st;
        if (stat(files.entries[i].path, &st) != 0) {
            printf("Failed to get file information for '%s'.\n", files.entries[i].path);
            continue;
        }

        if (st.st_mtime <= files.entries[i].last_modified) {
            printf("File '%s' is alerady modified.\n", files.entries[i].path);
            return;
        }
    }

    // 수정을 희망하는 version을 포인팅
    char ver_d[MAX_PATH_LENGTH];
    snprintf(ver_d, sizeof(ver_d), ".keep/%d", version);

    char target_d[MAX_PATH_LENGTH];
    snprintf(target_d, sizeof(target_d), "%s/target", ver_d);

    DIR* ver_d_check = opendir(ver_d);
    if (ver_d_check == NULL) {
        printf("version directory is not exist.\n");
        return;
    }

    while ((entry = readdir(ver_d_check)) != NULL) {
        if (entry->d_type == DT_REG) {
            char source_path[MAX_PATH_LENGTH];
            snprintf(source_path, sizeof(source_path), "%s/%s", target_d, entry->d_name);
            char dest_path[MAX_PATH_LENGTH];
            snprintf(dest_path, sizeof(dest_path), "%s/%s", ".", entry->d_name);
            if (rename(source_path, dest_path) != 0) {
                printf("Failed to restore file '%s'.\n", entry->d_name);
            }
        }
    }
    closedir(ver_d_check);

    FILE* latest_info = fopen(".keep/tracking-files", "w");
    if (latest_info == NULL) {
        printf("'tracking-files' is not exist.\n");
        return;
    }
    for (int i = 0; i < files.count; i++) {
        fprintf(latest_info, "%s %ld\n", files.entries[i].path, files.entries[i].last_modified);
    }
    fclose(latest_info);

    printf("Restored as version %d.\n", version);
}

int main(int argc, char ** argv) {
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
        // keep track <file or directory>
        addFile(arg);
    } else if (strcmp(command, "untrack") == 0) {
        // keep untrack <file or directory>
        removeFile(arg);
    } else if (strcmp(command, "versions") == 0) {
        // keep versions
        printVersions();
    } else if (strcmp(command, "store") == 0) {
        // keep store "note"
        store(arg);
    } else if (strcmp(command, "restore") == 0) {
        // keep restore <version>
        int version = atoi(arg);
        restore(version);
    } else {
        printf("Command not found.\n");
    }

    return 0;
}
