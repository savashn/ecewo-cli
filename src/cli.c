#include "cli.h"

Plugin plugins[] = {
    {"cJSON", CJSON_C_URL, CJSON_H_URL, 0},
    {"dotenv", DOTENV_C_URL, DOTENV_H_URL, 0},
    {"session", SESSION_C_URL, SESSION_H_URL, 0},
    {"pquv", PQUV_C_URL, PQUV_H_URL, 0},
    {"slugify", SLUGIFY_C_URL, SLUGIFY_H_URL, 0},
    {"SQLite3", SQLITE_C_URL, SQLITE_H_URL, 0},
    {"Postgres", NULL, NULL, 0},
    {"CBOR", NULL, NULL, 0},
};

const int plugin_count = sizeof(plugins) / sizeof(Plugin);

static int install_vendor(const char *plugin_name, const char *c_url, const char *h_url)
{
    if (!plugin_name || !c_url || !h_url)
        return -1;

    const char *target_dir = "vendors";

    printf("Installing %s...\n", plugin_name);

    if (create_directory(target_dir) != 0)
    {
        printf("The target directory couldn't be created: %s\n", target_dir);

        return -1;
    }

    // Calculate path sizes
    size_t path_size = strlen("vendors") + strlen(PATH_SEPARATOR) + strlen(plugin_name) + strlen(".c") + 1;
    char *c_path = malloc(path_size);
    char *h_path = malloc(path_size);

    if (!c_path || !h_path)
    {

        if (c_path)
            free(c_path);
        if (h_path)
            free(h_path);
        return -1;
    }

    build_vendor_path(c_path, path_size, plugin_name, "c");
    build_vendor_path(h_path, path_size, plugin_name, "h");

    if (download_file(c_url, c_path) != 0)
    {
        printf("Failed to download %s.c\n", plugin_name);

        free(c_path);
        free(h_path);
        return -1;
    }

    if (download_file(h_url, h_path) != 0)
    {
        printf("Failed to download %s.h\n", plugin_name);

        free(c_path);
        free(h_path);
        return -1;
    }

    printf("Files downloaded to %s\n", target_dir);

    // Update CMakeLists.txt
    char *cmake_content = read_file("CMakeLists.txt");
    if (!cmake_content)
    {
        printf("Warning: CMakeLists.txt not found, skipping CMake update.\n");

        free(c_path);
        free(h_path);
        return 0;
    }

    // Check if this vendor is already in CMake
    if (contains_string(cmake_content, plugin_name))
    {
        printf("%s is already in CMakeLists.txt\n", plugin_name);
        free(cmake_content);

        free(c_path);
        free(h_path);
        return 0;
    }

    // Get executable name dynamically
    char *exec_name = get_exec_name();
    if (!exec_name)
    {
        printf("Error: Could not determine executable name from CMakeLists.txt\n");
        free(cmake_content);

        free(c_path);
        free(h_path);
        return -1;
    }

    StringBuilder *sb = sb_create();
    if (!sb)
    {
        free(cmake_content);
        free(exec_name);

        free(c_path);
        free(h_path);
        return -1;
    }

    sb_append(sb, cmake_content);

    // Check if vendors include directory already exists
    if (!contains_string(cmake_content, "target_include_directories") ||
        !contains_string(cmake_content, "vendors"))
    {
        // Add vendors include directory (only once)
        sb_append(sb, "\n# Vendors include directory\n");
        sb_append(sb, "target_include_directories(");
        sb_append(sb, exec_name);
        sb_append(sb, " PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/vendors)\n");
    }

    // Add vendor-specific source file
    sb_append(sb, "\n# ");
    sb_append(sb, plugin_name);
    sb_append(sb, "\n");
    sb_append(sb, "target_sources(");
    sb_append(sb, exec_name);
    sb_append(sb, " PRIVATE vendors/");
    sb_append(sb, plugin_name);
    sb_append(sb, ".c)\n");

    // Write updated file
    int write_result = write_file("CMakeLists.txt", sb->data);

    // Cleanup
    free(cmake_content);
    free(exec_name);

    free(c_path);
    free(h_path);
    sb_free(sb);

    if (write_result != 0)
    {
        printf("Error updating CMakeLists.txt\n");
        return -1;
    }

    printf("%s installed and added to CMakeLists.txt successfully!\n", plugin_name);
    return 0;
}

static int uninstall_vendor(const char *plugin_name)
{
    if (!plugin_name)
        return -1;

    // Calculate path sizes
    size_t path_size = strlen("vendors") + strlen(PATH_SEPARATOR) + strlen(plugin_name) + strlen(".c") + 1;
    char *c_path = malloc(path_size);
    char *h_path = malloc(path_size);

    if (!c_path || !h_path)
    {
        if (c_path)
            free(c_path);
        if (h_path)
            free(h_path);
        return -1;
    }

    build_vendor_path(c_path, path_size, plugin_name, "c");
    build_vendor_path(h_path, path_size, plugin_name, "h");

    printf("Uninstalling %s...\n", plugin_name);

    int removed_any = 0;

    if (file_exists(c_path))
    {
        if (remove(c_path) == 0)
        {
            printf("Deleted: %s\n", c_path);
            removed_any++;
        }
        else
        {
            perror("Deletion Error: .c file couldn't be deleted");
        }
    }

    if (file_exists(h_path))
    {
        if (remove(h_path) == 0)
        {
            printf("Deleted: %s\n", h_path);
            removed_any++;
        }
        else
        {
            perror("Deletion Error: .h file couldn't be deleted");
        }
    }

    // Remove from CMakeLists.txt (only the source file, keep include directory)
    char *cmake_content = read_file("CMakeLists.txt");
    if (cmake_content)
    {
        char *result = malloc(strlen(cmake_content) + 1);
        if (result)
        {
            strcpy(result, cmake_content);

            // Remove only the plugin-specific support block
            size_t comment_size = strlen("# ") + strlen(plugin_name) + strlen("\n") + 1;
            char *comment_start = malloc(comment_size);
            if (comment_start)
            {
                snprintf(comment_start, comment_size, "# %s\n", plugin_name);
                char *block_start = strstr(result, comment_start);

                if (block_start)
                {
                    // Find end of this plugin's block (next comment or end of file)
                    char *block_end = strstr(block_start + strlen(comment_start), "\n# ");
                    if (!block_end)
                    {
                        // If no next comment, go to end of file
                        block_end = result + strlen(result);
                    }

                    memmove(block_start, block_end, strlen(block_end) + 1);
                }

                // Clean up extra newlines
                char *triple_newline;
                while ((triple_newline = strstr(result, "\n\n\n")) != NULL)
                {
                    memmove(triple_newline, triple_newline + 1, strlen(triple_newline));
                }

                write_file("CMakeLists.txt", result);
                free(comment_start);
            }
            free(result);
        }
        free(cmake_content);
    }

    free(c_path);
    free(h_path);

    printf("%s uninstalled successfully. %d files removed.\n", plugin_name, removed_any);
    return 0;
}

// Parse command line arguments - no changes needed, already safe
static void parse_arguments(int argc, char *argv[], flags_t *flags)
{
    memset(flags, 0, sizeof(flags_t));

    for (int i = 1; i < argc; i++)
    {
        // Project
        if (strcmp(argv[i], "run") == 0)
            flags->run = 1;
        else if (strcmp(argv[i], "create") == 0)
            flags->create = 1;
        else if (strcmp(argv[i], "rebuild") == 0)
            flags->rebuild = 1;
        else if (strcmp(argv[i], "build-prod") == 0)
            flags->build_production = 1;
        else if (strcmp(argv[i], "libs") == 0)
            flags->libs = 1;
        else if (strcmp(argv[i], "install") == 0)
            flags->install = 1;
        else if (strcmp(argv[i], "uninstall") == 0)
            flags->uninstall = 1;

        // Libraries
        else if (strcmp(argv[i], "cjson") == 0)
            flags->cjson = 1;
        else if (strcmp(argv[i], "dotenv") == 0)
            flags->dotenv = 1;
        else if (strcmp(argv[i], "sqlite") == 0)
            flags->sqlite = 1;
        else if (strcmp(argv[i], "postgres") == 0)
            flags->postgres = 1;
        else if (strcmp(argv[i], "pquv") == 0)
            flags->pquv = 1;
        else if (strcmp(argv[i], "session") == 0)
            flags->session = 1;
        else if (strcmp(argv[i], "slugify") == 0)
            flags->slugify = 1;
        else if (strcmp(argv[i], "cbor") == 0)
            flags->cbor = 1;
        else
            printf("Unknown argument: %s\n", argv[i]);
    }
}

static int create_project(void)
{
    // Get project name from user
    size_t project_name_size = 256;
    char *project_name = malloc(project_name_size);
    if (!project_name)
        return -1;

    printf("Create a project:\n");
    printf("Enter the project name >>> ");
    fflush(stdout);

    if (fgets(project_name, project_name_size, stdin) == NULL)
    {
        printf("Error reading project name\n");
        free(project_name);
        return -1;
    }

    // Remove newline character
    size_t len = strlen(project_name);
    if (len > 0 && project_name[len - 1] == '\n')
    {
        project_name[len - 1] = '\0';
    }

    if (strlen(project_name) == 0)
    {
        printf("Project name cannot be empty\n");
        free(project_name);
        return -1;
    }

    // Select plugin
    interactive_select();

    printf("Creating project: %s\n", project_name);

    // Create src directory
    const char *src_dir = "src";
    if (create_directory(src_dir) != 0)
    {
        printf("Error creating src directory\n");
        free(project_name);
        return -1;
    }

    const char *main_c =
        "#include \"server.h\"\n"
        "#include \"ecewo.h\"\n"
        "\n"
        "void destroy_app()\n"
        "{\n"
        "   reset_router();\n"
        "}\n"
        "\n"
        "int main()\n"
        "{\n"
        "   init_router();\n"
        "   shutdown_hook(destroy_app);\n"
        "   ecewo(3000);\n"
        "   return 0;\n"
        "}\n";

    if (write_file("src/main.c", main_c) != 0)
    {
        printf("Error creating main.c\n");
        free(project_name);
        return -1;
    }

    // Create CMakeLists.txt with project name
    size_t cmake_size = 1024 + strlen(project_name) * 3; // 3 times for 3 occurrences
    char *cmake_content = malloc(cmake_size);
    if (!cmake_content)
    {
        free(project_name);
        return -1;
    }

    snprintf(cmake_content, cmake_size,
             "cmake_minimum_required(VERSION 3.14)\n"
             "project(%s VERSION 0.1.0 LANGUAGES C)\n"
             "\n"
             "include(FetchContent)\n"
             "\n"
             "FetchContent_Declare(\n"
             "   ecewo\n"
             "   GIT_REPOSITORY https://github.com/savashn/ecewo.git\n"
             "   GIT_TAG main\n"
             ")\n"
             "\n"
             "FetchContent_MakeAvailable(ecewo)\n"
             "\n"
             "add_executable(%s\n"
             "  src/main.c\n"
             ")\n"
             "\n"
             "target_link_libraries(%s PRIVATE ecewo)\n",
             project_name, project_name, project_name);

    if (write_file("CMakeLists.txt", cmake_content) != 0)
    {
        printf("Error creating CMakeLists.txt\n");
        free(project_name);
        free(cmake_content);
        return -1;
    }

    // Install selected plugins
    for (int i = 0; i < plugin_count; i++)
    {
        if (plugins[i].selected)
        {
            // Specific processes only for cbor and postgres
            if (strcmp(plugins[i].name, "CBOR") == 0)
            {
                printf("Handling TinyCBOR integration...\n");
                install_cbor();
                continue;
            }

            if (strcmp(plugins[i].name, "Postgres") == 0)
            {
                printf("Handling postgres integration...\n");
                install_postgres();
                continue;
            }

            // Installing process for the other plugins
            if (install_vendor(
                    plugins[i].name,
                    plugins[i].c_url,
                    plugins[i].h_url) != 0)
            {
                fprintf(stderr, "Error installing %s\n", plugins[i].name);
            }
        }
    }

    printf("Starter project created successfully.\n");
    printf("Project '%s' created successfully!\n", project_name);
    printf("To build and run your project:\n");
    printf("ecewo run\n");

    free(project_name);
    free(cmake_content);
    return 0;
}

// Build and run function
static int build_and_run(void)
{
    printf("Creating build directory...\n");
    create_directory("build");

    if (chdir("build") != 0)
    {
        printf("Error: Cannot change to build directory\n");
        return -1;
    }

    printf("Configuring with CMake...\n");
    if (execute_command("cmake ..") != 0)
    {
        printf("Error: cmake configuration failed\n");
        chdir("..");
        return -1;
    }

    printf("Building...\n");
    if (execute_command("cmake --build .") != 0)
    {
        printf("Error: Build failed\n");
        chdir("..");
        return -1;
    }

    printf("Build completed!\n\n");
    printf("Running server...\n");

    // Get executable name dynamically
    chdir("..");
    char *exec_name = get_exec_name();
    chdir("build");

    if (exec_name)
    {
        // Platform-specific executable path ve extension
        size_t exec_path_size = strlen(exec_name) + 10; // Extra space for extension
        char *exec_path = malloc(exec_path_size);
        if (exec_path)
        {

#ifdef _WIN32
            // Windows: önce .exe ile dene
            snprintf(exec_path, exec_path_size, "%s.exe", exec_name); // ./ kaldırıldı
            if (file_exists(exec_path))
            {
                execute_command(exec_path);
            }
            else
            {
                snprintf(exec_path, exec_path_size, "%s", exec_name);
                if (file_exists(exec_path))
                {
                    execute_command(exec_path);
                }
                else
                {
                    printf("Executable %s not found (tried %s.exe and %s). Check for build errors.\n",
                           exec_name, exec_name, exec_name);
                }
            }
#else
            // Linux/macOS: ./ ile
            snprintf(exec_path, exec_path_size, "./%s", exec_name);
            if (file_exists(exec_path))
            {
                execute_command(exec_path);
            }
            else
            {
                printf("Executable %s not found. Check for build errors.\n", exec_name);
            }
#endif
            free(exec_path);
        }
        free(exec_name);
    }
    else
    {
        printf("Could not determine executable name. Check CMakeLists.txt.\n");
    }

    chdir("..");
    return 0;
}

static int build_production(void)
{
    printf("Creating production build (Release mode)...\n");

    const char *build_dir = "build-prod";
    if (create_directory(build_dir) != 0)
    {
        printf("Error creating build directory\n");
        return -1;
    }

    if (chdir(build_dir) != 0)
    {
        printf("Error: Cannot change to build directory\n");
        return -1;
    }

    printf("Configuring with CMake (Release)...\n");
    if (execute_command("cmake -DCMAKE_BUILD_TYPE=Release ..") != 0)
    {
        printf("Error: cmake configuration failed\n");
        chdir("..");
        return -1;
    }

    printf("Building (Release)...\n");
    if (execute_command("cmake --build . --config Release") != 0)
    {
        printf("Error: Build failed\n");
        chdir("..");
        return -1;
    }

    printf("Production build completed!\n");
    printf("Optimized binary location: %s/\n", build_dir);

    chdir("..");
    return 0;
}

int main(int argc, char *argv[])
{
    printf("Ecewo CLI\n");
    printf("2025 (c) Savas Sahin <savashn>\n\n");

    flags_t flags;
    parse_arguments(argc, argv, &flags);

    // Check if no parameters were provided
    if ((!flags.create && !flags.run && !flags.rebuild && !flags.build_production && !flags.libs && !flags.install && !flags.uninstall) || (flags.help))
    {
        show_help();
        return 0;
    }

    // Handle create command
    if (flags.create)
    {
        return create_project();
    }

    // Handle run command
    if (flags.run)
    {
        return build_and_run();
    }

    // Handle rebuild command
    if (flags.rebuild)
    {
        printf("Cleaning build directory...\n");
        remove_directory("build");
        printf("Rebuilding...\n\n");
        return build_and_run();
    }

    if (flags.build_production)
    {
        return build_production();
    }

    if (flags.libs)
    {
        show_install_help();
        return 0;
    }

    if (flags.uninstall)
    {
        int has_plugin_arg = flags.cjson || flags.dotenv || flags.sqlite || flags.pquv || flags.session || flags.slugify || flags.cbor || flags.postgres;

        if (!has_plugin_arg)
        {
            show_install_help();
            return 0;
        }

        if (flags.cbor)
            uninstall_cbor();

        if (flags.postgres)
            uninstall_postgres();

        if (flags.cjson)
            uninstall_vendor("cJSON");

        if (flags.dotenv)
            uninstall_vendor("dotenv");

        if (flags.session)
            uninstall_vendor("session");

        if (flags.pquv)
            uninstall_vendor("pquv");

        if (flags.slugify)
            uninstall_vendor("slugify");

        if (flags.sqlite)
            uninstall_vendor("sqlite");

        return 0;
    }

    // Handle install command
    if (flags.install)
    {
        int has_plugin_arg = flags.cjson || flags.dotenv || flags.sqlite || flags.session || flags.cbor || flags.slugify || flags.pquv || flags.postgres;

        if (!has_plugin_arg)
        {
            show_install_help();
            return 0;
        }

        if (flags.cbor)
            install_cbor();

        if (flags.postgres)
            install_postgres();

        if (flags.cjson)
            install_vendor("cJSON", CJSON_C_URL, CJSON_H_URL);

        if (flags.dotenv)
            install_vendor("dotenv", DOTENV_C_URL, DOTENV_H_URL);

        if (flags.session)
            install_vendor("session", SESSION_C_URL, SESSION_H_URL);

        if (flags.pquv)
            install_vendor("pquv", PQUV_C_URL, PQUV_H_URL);

        if (flags.slugify)
            install_vendor("slugify", SLUGIFY_C_URL, SLUGIFY_H_URL);

        if (flags.sqlite)
            install_vendor("sqlite", SQLITE_C_URL, SQLITE_H_URL);

        return 0;
    }

    return 0;
}
