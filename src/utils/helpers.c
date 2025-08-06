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
    printf("  ecewo create          # Create a new Ecewo project\n");
    printf("  ecewo run             # Build and run the project\n");
    printf("  ecewo build dev       # Build for development\n");
    printf("  ecewo build prod      # Build for production\n");
    printf("  ecewo rebuild dev     # Clean and rebuild for development\n");
    printf("  ecewo rebuild prod    # Clean and rebuild for production\n");
    printf("  ecewo libs            # See library installation commands\n");
    printf("  ecewo install [lib]   # Install a library\n");
    printf("  ecewo uninstall [lib] # Uninstall a library\n");
    printf("==========================================================\n");
}
