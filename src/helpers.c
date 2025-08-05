#include <stdio.h>

void show_install_help(void)
{
    printf("Libraries:\n");
    printf("=============================================\n");
    printf("  JSON          ecewo install cjson\n");
    printf("  CBOR          ecewo install cbor\n");
    printf("  .env          ecewo install dotenv\n");
    printf("  SQLite3       ecewo install sqlite\n");
    printf("  PostgreSQL    ecewo install postgres\n");
    printf("  Session       ecewo install session\n");
    printf("  PQUV          ecewo install pquv\n");
    printf("  Slugify       ecewo install slugify\n");
    printf("=============================================\n");
}

void show_help(void)
{
    printf("No parameters specified. Please use one of the following:\n");
    printf("==========================================================\n");
    printf("  ecewo create      # Create a new Ecewo project\n");
    printf("  ecewo run         # Build and run the project (dev)\n");
    printf("  ecewo rebuild     # Build from scratch and run (dev)\n");
    printf("  ecewo build-prod  # Build for production\n");
    printf("  ecewo libs        # Library installation commands\n");
    printf("  ecewo install     # Install libraries\n");
    printf("  ecewo uninstall   # Uninstall libraries\n");
    printf("==========================================================\n");
}
