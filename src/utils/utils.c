#include "cli.h"

StringBuilder *sb_create(void)
{
    StringBuilder *sb = malloc(sizeof(StringBuilder));
    if (!sb)
        return NULL;

    sb->capacity = 256;
    sb->size = 0;
    sb->data = malloc(sb->capacity);
    if (!sb->data)
    {
        free(sb);
        return NULL;
    }
    sb->data[0] = '\0';
    return sb;
}

void sb_append(StringBuilder *sb, const char *str)
{
    if (!sb || !str)
        return;

    size_t len = strlen(str);
    while (sb->size + len + 1 > sb->capacity)
    {
        sb->capacity *= 2;
        char *new_data = realloc(sb->data, sb->capacity);
        if (!new_data)
            return; // Handle realloc failure
        sb->data = new_data;
    }
    strcat(sb->data, str);
    sb->size += len;
}

void sb_free(StringBuilder *sb)
{
    if (sb)
    {
        if (sb->data)
            free(sb->data);
        free(sb);
    }
}

int file_exists(const char *path)
{
    return access(path, F_OK) == 0;
}

int directory_exists(const char *path)
{
    struct stat st;
    return (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
}

int create_directory(const char *path)
{
    if (!path)
        return -1;

    size_t path_len = strlen(path);
    char *tmp = malloc(path_len + 1);
    if (!tmp)
        return -1;

    strcpy(tmp, path);

    // Separate according to PATH_SEPARATOR
    for (size_t i = 1; i < path_len; i++)
    {
        if (tmp[i] == '/' || tmp[i] == '\\')
        {
            tmp[i] = '\0';
            if (access(tmp, F_OK) != 0)
            {
                if (mkdir(tmp, 0755) != 0)
                {
                    free(tmp);
                    return -1;
                }
            }
            tmp[i] = PATH_SEPARATOR[0];
        }
    }

    // Create the last directory
    int result = 0;
    if (access(tmp, F_OK) != 0)
    {
        if (mkdir(tmp, 0755) != 0)
            result = -1;
    }

    free(tmp);
    return result;
}

int remove_directory(const char *path)
{
    if (!path)
        return -1;

    // Calculate needed size: command template + path + null terminator
    size_t cmd_size;
#ifdef _WIN32
    cmd_size = strlen("rmdir /s /q \"\" 2>nul") + strlen(path) + 1;
#else
    cmd_size = strlen("rm -rf \"\"") + strlen(path) + 1;
#endif

    char *command = malloc(cmd_size);
    if (!command)
        return -1;

#ifdef _WIN32
    snprintf(command, cmd_size, "rmdir /s /q \"%s\" 2>nul", path);
#else
    snprintf(command, cmd_size, "rm -rf \"%s\"", path);
#endif

    int result = system_command(command);
    free(command);
    return result;
}

int execute_command(const char *command)
{
    if (!command)
        return -1;

    printf("Executing: %s\n", command);
    return system_command(command);
}

int download_file(const char *url, const char *output_path)
{
    if (!url || !output_path)
        return -1;

    // Calculate needed size
    size_t cmd_size = strlen("curl -o \"\" \"\"") + strlen(output_path) + strlen(url) + 1;
    char *command = malloc(cmd_size);
    if (!command)
        return -1;

    snprintf(command, cmd_size, "curl -o \"%s\" \"%s\"", output_path, url);
    int result = execute_command(command);
    free(command);
    return result;
}

// Check if string contains substring
int contains_string(const char *haystack, const char *needle)
{
    if (!haystack || !needle)
        return 0;
    return strstr(haystack, needle) != NULL;
}

// Read file content
char *read_file(const char *filename)
{
    if (!filename)
        return NULL;

    FILE *file = fopen(filename, "r");
    if (!file)
    {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    if (length < 0)
    {
        fclose(file);
        return NULL;
    }
    fseek(file, 0, SEEK_SET);

    char *content = malloc(length + 1);
    if (!content)
    {
        fclose(file);
        return NULL;
    }

    size_t read_size = fread(content, 1, length, file);
    content[read_size] = '\0';
    fclose(file);

    return content;
}

// Write file content
int write_file(const char *filename, const char *content)
{
    if (!filename || !content)
        return -1;

    FILE *file = fopen(filename, "w");
    if (!file)
    {
        return -1;
    }

    if (fputs(content, file) == EOF)
    {
        fclose(file);
        return -1;
    }

    fclose(file);
    return 0;
}

// Get executable name from CMakeLists.txt
char *get_exec_name(void)
{
    char *cmake_content = read_file("CMakeLists.txt");
    if (!cmake_content)
    {
        return NULL;
    }

    // Find add_executable line
    char *exec_line = strstr(cmake_content, "add_executable(");
    if (!exec_line)
    {
        free(cmake_content);
        return NULL;
    }

    // Move to after "add_executable("
    exec_line += strlen("add_executable(");

    // Find the executable name (until space or newline)
    char *name_end = exec_line;
    while (*name_end && *name_end != ' ' && *name_end != '\n' && *name_end != '\t')
    {
        name_end++;
    }

    // Copy the name
    size_t name_len = name_end - exec_line;
    char *exec_name = malloc(name_len + 1);
    if (!exec_name)
    {
        free(cmake_content);
        return NULL;
    }

    strncpy(exec_name, exec_line, name_len);
    exec_name[name_len] = '\0';

    free(cmake_content);
    return exec_name;
}

// Platform-agnostic path building
void build_vendor_path(char *buffer, size_t buffer_size, const char *plugin_name, const char *extension)
{
    if (!buffer || !plugin_name || !extension)
        return;

    snprintf(buffer, buffer_size,
             "vendors%s%s.%s",
             PATH_SEPARATOR,
             plugin_name,
             extension);
}
