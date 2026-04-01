#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <direct.h>     // Windows mkdir

// maps file extensions to folder names
typedef struct {
    char *extension;
    char *folder;
} FileRule;

// forward declaration so get_folder can be called from main
char *get_folder(char *ext);

FileRule rules[] = {
    {"jpg",  "images"},
    {"jpeg", "images"},
    {"png",  "images"},
    {"gif",  "images"},
    {"bmp",  "images"},
    {"svg",  "images"},
    {"pdf",  "documents"},
    {"txt",  "documents"},
    {"docx", "documents"},
    {"doc",  "documents"},
    {"xlsx", "documents"},
    {"pptx", "documents"},
    {"c",    "code"},
    {"h",    "code"},
    {"py",   "code"},
    {"js",   "code"},
    {"html", "code"},
    {"css",  "code"},
    {"java", "code"},
    {"cpp",  "code"},
    {"mp4",  "videos"},
    {"mov",  "videos"},
    {"avi",  "videos"},
    {"mkv",  "videos"},
    {"mp3",  "audio"},
    {"wav",  "audio"},
    {"flac", "audio"},
    {NULL,   NULL}
};

// returns everything after the last dot in a filename
char *get_extension(char *filename) {
    char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) return "";
    return dot + 1;
}

// looks up the folder name for a given extension
char *get_folder(char *ext) {
    for (int i = 0; rules[i].extension != NULL; i++) {
        if (strcmp(ext, rules[i].extension) == 0)
            return rules[i].folder;
    }
    return "other";
}

// creates a folder, does nothing if it already exists
void create_folder(char *path) {
    _mkdir(path);
}

// moves a file into a folder, skips if destination already exists
int move_file(char *filename, char *folder) {
    char destination[512];
    snprintf(destination, sizeof(destination), "%s/%s", folder, filename);

    FILE *test = fopen(destination, "r");
    if (test != NULL) {
        fclose(test);
        printf("Skipped  %s (already exists in %s/)\n", filename, folder);
        return 0;
    }

    if (rename(filename, destination) == 0) {
        printf("Moved    %s -> %s/\n", filename, folder);
        return 1;
    } else {
        printf("Failed   %s (could not move)\n", filename);
        return 0;
    }
}

int main(int argc, char *argv[]) {
    char *target = (argc > 1) ? argv[1] : "."; // use provided path or current dir

    DIR *dir = opendir(target);
    if (dir == NULL) {
        printf("Error: could not open directory \"%s\"\n", target);
        return 1;
    }

    if (chdir(target) != 0) {
        printf("Error: could not change into directory \"%s\"\n", target);
        closedir(dir);
        return 1;
    }

    struct dirent *entry;
    int moved = 0, skipped = 0;

    printf("\nOrganising \"%s\"...\n\n", target);

    while ((entry = readdir(dir)) != NULL) {
        char *name = entry->d_name;

        if (name[0] == '.') continue;          // skip hidden files and . / ..

        struct stat path_stat;
        stat(name, &path_stat);
        if (S_ISDIR(path_stat.st_mode)) continue; // skip subdirectories

        char *ext = get_extension(name);
        if (strlen(ext) == 0) continue;        // skip files with no extension

        // convert extension to lowercase for matching
        char ext_lower[32];
        strncpy(ext_lower, ext, sizeof(ext_lower) - 1);
        ext_lower[sizeof(ext_lower) - 1] = '\0';
        for (int i = 0; ext_lower[i]; i++)
            ext_lower[i] = (char)tolower((unsigned char)ext_lower[i]);

        char *folder = get_folder(ext_lower);
        create_folder(folder);

        if (move_file(name, folder))
            moved++;
        else
            skipped++;
    }

    closedir(dir);
    printf("\nDone! %d file(s) moved, %d skipped.\n", moved, skipped);
    return 0;
}