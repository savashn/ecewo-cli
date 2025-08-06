CC = gcc 
 
CFLAGS = -Wall -Wextra -std=c99 -Isrc
 
ifeq ($(OS),Windows_NT)
    TARGET = ecewo.exe
    INSTALL_DIR = $(HOME)/bin
    INSTALL_NAME = ecewo.exe
else
    TARGET = ecewo 
    INSTALL_DIR = $(HOME)/.local/bin
    INSTALL_NAME = ecewo
endif
 
SRCS = src/cli.c src/utils/select_menu.c src/utils/utils.c src/utils/helpers.c src/lib/cbor.c src/lib/postgres.c
 
all: $(TARGET) 
 
$(TARGET): $(SRCS) src/cli.h
	$(CC) $(CFLAGS) -o $@ $(SRCS)

install: $(TARGET)
	@echo "Installing $(TARGET) to $(INSTALL_DIR)..."
	@mkdir -p $(INSTALL_DIR)
	@cp $(TARGET) $(INSTALL_DIR)/$(INSTALL_NAME)
ifeq ($(OS),Windows_NT)
	@echo "Ecewo CLI installed successfully!"
else
	@chmod +x $(INSTALL_DIR)/$(INSTALL_NAME)
	@echo "Ecewo CLI installed successfully!"
endif

uninstall:
	@echo "Uninstalling Ecewo CLI..."
	@rm -f $(INSTALL_DIR)/$(INSTALL_NAME)
	@echo "Ecewo CLI uninstalled successfully!"
	@rm -f $(TARGET)

clean: 
	rm -f $(TARGET)

help:
	@echo "Available targets:"
	@echo "  all       - Build Ecewo CLI"
	@echo "  install   - Install Ecewo CLI to PATH"
	@echo "  uninstall - Remove Ecewo CLI from PATH"
	@echo "  clean     - Remove build files"
	@echo "  help      - Show this help"

.PHONY: all install uninstall clean help
