#include "cli.h"

int install_postgres(void)
{
    printf("Installing PostgreSQL...\n");

    char *cmake_content = read_file("CMakeLists.txt");
    if (!cmake_content)
    {
        printf("Error: CMakeLists.txt not found.\n");
        return -1;
    }

    if (contains_string(cmake_content, "PostgreSQL"))
    {
        printf("PostgreSQL is already installed.\n");
        free(cmake_content);
        return 0;
    }

    // Get executable name dynamically
    char *exec_name = get_exec_name();
    if (!exec_name)
    {
        printf("Error: Could not determine executable name from CMakeLists.txt\n");
        free(cmake_content);
        return -1;
    }

    StringBuilder *sb = sb_create();
    if (!sb)
    {
        free(cmake_content);
        free(exec_name);
        return -1;
    }

    sb_append(sb, cmake_content);

    // Add PostgreSQL configuration
    sb_append(sb, "\n# PostgreSQL\n");
    sb_append(sb, "find_package(PostgreSQL REQUIRED)\n");
    sb_append(sb, "target_include_directories(");
    sb_append(sb, exec_name);
    sb_append(sb, " PRIVATE ${PostgreSQL_INCLUDE_DIRS})\n");
    sb_append(sb, "target_link_libraries(");
    sb_append(sb, exec_name);
    sb_append(sb, " PRIVATE ${PostgreSQL_LIBRARIES})\n");

    if (write_file("CMakeLists.txt", sb->data) != 0)
    {
        printf("Error writing CMakeLists.txt\n");
        free(cmake_content);
        free(exec_name);
        sb_free(sb);
        return -1;
    }

    printf("PostgreSQL installed successfully!\n");
    free(cmake_content);
    free(exec_name);
    sb_free(sb);
    return 0;
}

int uninstall_postgres(void)
{
    printf("Uninstalling PostgreSQL...\n");

    char *cmake_content = read_file("CMakeLists.txt");
    if (!cmake_content)
    {
        printf("Error: CMakeLists.txt not found.\n");
        return -1;
    }

    if (!contains_string(cmake_content, "PostgreSQL"))
    {
        printf("PostgreSQL is not installed.\n");
        free(cmake_content);
        return 0;
    }

    char *result = malloc(strlen(cmake_content) + 1);
    if (!result)
    {
        free(cmake_content);
        return -1;
    }
    strcpy(result, cmake_content);

    // Remove entire PostgreSQL block (from comment to target_link_libraries)
    char *pg_start = strstr(result, "# PostgreSQL\n");
    if (pg_start)
    {
        // Get executable name to build the correct target_link_libraries line
        char *exec_name = get_exec_name();
        if (exec_name)
        {
            // Build the end string dynamically
            size_t end_str_size = strlen("target_link_libraries(") + strlen(exec_name) + strlen(" PRIVATE ${PostgreSQL_LIBRARIES})\n") + 1;
            char *end_str = malloc(end_str_size);
            if (end_str)
            {
                snprintf(end_str, end_str_size, "target_link_libraries(%s PRIVATE ${PostgreSQL_LIBRARIES})\n", exec_name);

                char *pg_end = strstr(pg_start, end_str);
                if (pg_end)
                {
                    pg_end += strlen(end_str);
                    memmove(pg_start, pg_end, strlen(pg_end) + 1);
                }
                free(end_str);
            }
            free(exec_name);
        }
    }

    // Clean up extra newlines
    char *triple_newline;
    while ((triple_newline = strstr(result, "\n\n\n")) != NULL)
    {
        memmove(triple_newline, triple_newline + 1, strlen(triple_newline));
    }

    if (write_file("CMakeLists.txt", result) != 0)
    {
        printf("Error writing CMakeLists.txt\n");
        free(cmake_content);
        free(result);
        return -1;
    }

    printf("PostgreSQL uninstalled successfully!\n");
    free(cmake_content);
    free(result);
    return 0;
}
