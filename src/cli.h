#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#include <conio.h>
#define PATH_SEPARATOR "\\"
#define mkdir(path, mode) _mkdir(path)
#define access(path, mode) _access(path, mode)
#define F_OK 0
#define system_command(cmd) system(cmd)

#else
#include <unistd.h>
#include <termios.h>
#define PATH_SEPARATOR "/"
#define system_command(cmd) system(cmd)
#endif

#define REPO_URL "https://github.com/savashn/ecewo"

#define CJSON_C_URL "https://raw.githubusercontent.com/DaveGamble/cJSON/master/cJSON.c"
#define CJSON_H_URL "https://raw.githubusercontent.com/DaveGamble/cJSON/master/cJSON.h"
#define DOTENV_C_URL "https://raw.githubusercontent.com/Isty001/dotenv-c/master/src/dotenv.c"
#define DOTENV_H_URL "https://raw.githubusercontent.com/Isty001/dotenv-c/master/src/dotenv.h"
#define SESSION_C_URL "https://raw.githubusercontent.com/savashn/ecewo-session/main/session.c"
#define SESSION_H_URL "https://raw.githubusercontent.com/savashn/ecewo-session/main/session.h"
#define SLUGIFY_C_URL "https://raw.githubusercontent.com/savashn/slugify-c/main/slugify.c"
#define SLUGIFY_H_URL "https://raw.githubusercontent.com/savashn/slugify-c/main/slugify.h"
#define PQUV_C_URL "https://raw.githubusercontent.com/savashn/pquv/main/pquv.c"
#define PQUV_H_URL "https://raw.githubusercontent.com/savashn/pquv/main/pquv.h"
#define SQLITE_C_URL "https://raw.githubusercontent.com/rhuijben/sqlite-amalgamation/master/sqlite3.c"
#define SQLITE_H_URL "https://raw.githubusercontent.com/rhuijben/sqlite-amalgamation/master/sqlite3.h"

typedef struct
{
    const char *name;
    const char *c_url;
    const char *h_url;
    int selected;
} Plugin;

extern Plugin plugins[];
extern const int plugin_count;

// Command flags
typedef struct
{
    int run;
    int build;
    int build_dev;
    int build_prod;
    int rebuild;
    int rebuild_dev;
    int rebuild_prod;
    int libs;
    int install;
    int uninstall;
    int create;
    int help;
    int cjson;
    int dotenv;
    int sqlite;
    int postgres;
    int session;
    int pquv;
    int slugify;
    int cbor;
} flags_t;

typedef struct
{
    char *data;
    size_t size;
    size_t capacity;
} StringBuilder;

// UTILS
int file_exists(const char *path);
int create_directory(const char *path);
int remove_directory(const char *path);
int download_file(const char *url, const char *output_path);
int execute_command(const char *command);
int write_file(const char *filename, const char *content);
StringBuilder *sb_create(void);
void sb_append(StringBuilder *sb, const char *str);
void sb_free(StringBuilder *sb);
int contains_string(const char *haystack, const char *needle);
char *read_file(const char *filename);
char *get_exec_name(void);
void build_vendor_path(char *buffer, size_t buffer_size, const char *plugin_name, const char *extension);

// SELECT MENU
void clear_screen(void);
void draw_menu(int current);
int read_key(void);
void interactive_select(void);

// HELPERS
void show_install_help(void);
void show_help(void);

// LIBRARIES
int install_cbor(void);
int uninstall_cbor(void);
int install_postgres(void);
int uninstall_postgres(void);
