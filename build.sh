#!/bin/bash

# OBSBOT Meet 2 Control - Build and Install Script
# This script helps you build and optionally install the application

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
INSTALL_DIR="${XDG_BIN_HOME:-$HOME/.local/bin}"
DESKTOP_DIR="${XDG_DATA_HOME:-$HOME/.local/share}/applications"
ICON_DIR="${XDG_DATA_HOME:-$HOME/.local/share}/icons/hicolor/scalable/apps"
BUILD_DIR="build"
PROJECT_NAME="obsbot-meet2"

# Print colored message
print_msg() {
    local color=$1
    shift
    echo -e "${color}$@${NC}"
}

# Show usage information
show_usage() {
    cat << EOF
${GREEN}OBSBOT Meet 2 Control - Build Script${NC}

${YELLOW}Usage:${NC}
  ./build.sh <command> [--confirm]

${YELLOW}Commands:${NC}
  ${BLUE}build${NC}
    Compiles the project in the build/ directory.

    What it does:
    - Creates build/ directory if it doesn't exist
    - Runs CMake to configure the project
    - Compiles both GUI and CLI applications
    - Binaries will be in build/obsbot-meet2-gui and build/obsbot-meet2-cli

    ${YELLOW}Example:${NC} ./build.sh build --confirm

  ${BLUE}install${NC}
    Builds the project and installs binaries to your local bin directory.

    What it does:
    - Runs the build process (as above)
    - Copies binaries to ${INSTALL_DIR}
    - Makes them executable
    - Installs desktop launcher (appears in your application menu)
    - Installs application icon
    - Checks if install directory is in your PATH
    - Offers to add to PATH if needed (optional, requires your approval)

    ${YELLOW}Example:${NC} ./build.sh install --confirm

  ${BLUE}clean${NC}
    Removes the build/ directory for a fresh start.

    ${YELLOW}Example:${NC} ./build.sh clean --confirm

  ${BLUE}help${NC}
    Shows this help message.

${YELLOW}Options:${NC}
  ${BLUE}--confirm${NC}
    Required flag to actually execute the command. Without this flag,
    the script will only show what it would do without making changes.

${YELLOW}Notes:${NC}
- Install directory: ${INSTALL_DIR}
- Build directory: ${BUILD_DIR}
- The script will prompt for confirmation before making PATH changes
- You can safely run commands without --confirm to see what will happen

EOF
}

# Check if directory is in PATH
check_path() {
    if [[ ":$PATH:" == *":$1:"* ]]; then
        return 0
    else
        return 1
    fi
}

# Offer to add directory to PATH
offer_path_update() {
    local dir=$1

    print_msg "$YELLOW" "\n⚠️  The install directory is not in your PATH:"
    print_msg "$BLUE" "   $dir"

    echo ""
    print_msg "$YELLOW" "To use the installed applications from anywhere, add this directory to your PATH."
    echo ""
    print_msg "$NC" "Add the following line to your shell configuration file:"
    echo ""
    print_msg "$GREEN" "  For Bash (add to ~/.bashrc):"
    print_msg "$BLUE" "    export PATH=\"\$HOME/.local/bin:\$PATH\""
    echo ""
    print_msg "$GREEN" "  For Zsh (add to ~/.zshrc):"
    print_msg "$BLUE" "    export PATH=\"\$HOME/.local/bin:\$PATH\""
    echo ""

    read -p "Would you like me to add this to your shell config automatically? [y/N] " -n 1 -r
    echo

    if [[ $REPLY =~ ^[Yy]$ ]]; then
        # Detect shell
        local shell_config=""
        if [ -n "$BASH_VERSION" ]; then
            shell_config="$HOME/.bashrc"
        elif [ -n "$ZSH_VERSION" ]; then
            shell_config="$HOME/.zshrc"
        else
            # Try to detect from SHELL variable
            case "$SHELL" in
                */bash)
                    shell_config="$HOME/.bashrc"
                    ;;
                */zsh)
                    shell_config="$HOME/.zshrc"
                    ;;
                *)
                    print_msg "$RED" "❌ Could not detect shell type. Please add to PATH manually."
                    return 1
                    ;;
            esac
        fi

        print_msg "$YELLOW" "\n📝 I will add the following line to $shell_config:"
        print_msg "$BLUE" "   export PATH=\"\$HOME/.local/bin:\$PATH\""
        echo ""
        read -p "Proceed? [y/N] " -n 1 -r
        echo

        if [[ $REPLY =~ ^[Yy]$ ]]; then
            # Create backup
            cp "$shell_config" "$shell_config.backup.$(date +%Y%m%d_%H%M%S)"
            print_msg "$GREEN" "✓ Created backup: $shell_config.backup.*"

            # Add PATH export
            echo "" >> "$shell_config"
            echo "# Added by OBSBOT Meet 2 Control install script" >> "$shell_config"
            echo "export PATH=\"\$HOME/.local/bin:\$PATH\"" >> "$shell_config"

            print_msg "$GREEN" "✓ Added to $shell_config"
            print_msg "$YELLOW" "\n⚠️  You need to restart your shell or run:"
            print_msg "$BLUE" "   source $shell_config"
        else
            print_msg "$YELLOW" "Skipped. Add to PATH manually when ready."
        fi
    else
        print_msg "$YELLOW" "Skipped. Add to PATH manually when ready."
    fi
}

# Check for build dependencies
check_dependencies() {
    print_msg "$BLUE" "🔍 Checking build dependencies..."
    echo ""

    local all_ok=true

    # Check for cmake
    if command -v cmake &> /dev/null; then
        local cmake_version=$(cmake --version | head -1 | awk '{print $3}')
        print_msg "$GREEN" "  ✓ CMake ($cmake_version)"
    else
        print_msg "$RED" "  ✗ CMake - NOT FOUND"
        print_msg "$YELLOW" "    Install: sudo apt install cmake"
        all_ok=false
    fi

    # Check for make
    if command -v make &> /dev/null; then
        print_msg "$GREEN" "  ✓ Make"
    else
        print_msg "$RED" "  ✗ Make - NOT FOUND"
        print_msg "$YELLOW" "    Install: sudo apt install build-essential"
        all_ok=false
    fi

    # Check for C++ compiler
    if command -v g++ &> /dev/null; then
        local gxx_version=$(g++ --version | head -1 | awk '{print $3}')
        print_msg "$GREEN" "  ✓ C++ Compiler (g++ $gxx_version)"
    elif command -v clang++ &> /dev/null; then
        local clang_version=$(clang++ --version | head -1 | awk '{print $3}')
        print_msg "$GREEN" "  ✓ C++ Compiler (clang++ $clang_version)"
    else
        print_msg "$RED" "  ✗ C++ Compiler - NOT FOUND"
        print_msg "$YELLOW" "    Install: sudo apt install build-essential"
        all_ok=false
    fi

    # Check for pkg-config
    if command -v pkg-config &> /dev/null; then
        print_msg "$GREEN" "  ✓ pkg-config"
    else
        print_msg "$RED" "  ✗ pkg-config - NOT FOUND"
        print_msg "$YELLOW" "    Install: sudo apt install pkg-config"
        all_ok=false
    fi

    # Check for Qt6 Core
    if pkg-config --exists Qt6Core 2>/dev/null; then
        local qt6_version=$(pkg-config --modversion Qt6Core)
        print_msg "$GREEN" "  ✓ Qt6 Core ($qt6_version)"
    else
        print_msg "$RED" "  ✗ Qt6 Core - NOT FOUND"
        print_msg "$YELLOW" "    Install: sudo apt install qt6-base-dev"
        all_ok=false
    fi

    # Check for Qt6 Widgets
    if pkg-config --exists Qt6Widgets 2>/dev/null; then
        print_msg "$GREEN" "  ✓ Qt6 Widgets"
    else
        print_msg "$RED" "  ✗ Qt6 Widgets - NOT FOUND"
        print_msg "$YELLOW" "    Install: sudo apt install qt6-base-dev"
        all_ok=false
    fi

    # Check for Qt6 Multimedia
    if pkg-config --exists Qt6Multimedia 2>/dev/null; then
        print_msg "$GREEN" "  ✓ Qt6 Multimedia"
    else
        print_msg "$RED" "  ✗ Qt6 Multimedia - NOT FOUND"
        print_msg "$YELLOW" "    Install: sudo apt install qt6-multimedia-dev"
        all_ok=false
    fi

    # Check for optional but recommended tools
    echo ""
    print_msg "$BLUE" "Optional dependencies:"

    if command -v lsof &> /dev/null; then
        print_msg "$GREEN" "  ✓ lsof (for camera usage detection)"
    else
        print_msg "$YELLOW" "  ⚠ lsof - NOT FOUND (optional, but recommended)"
        print_msg "$BLUE" "    Install: sudo apt install lsof"
    fi

    echo ""
    if [ "$all_ok" = true ]; then
        print_msg "$GREEN" "✓ All required dependencies are installed!"
        return 0
    else
        print_msg "$RED" "✗ Some required dependencies are missing."
        print_msg "$YELLOW" "\nPlease install the missing packages and try again."
        return 1
    fi
}

# Build the project
do_build() {
    # Check dependencies first
    if ! check_dependencies; then
        echo ""
        print_msg "$RED" "❌ Cannot build: missing dependencies"
        exit 1
    fi

    echo ""
    print_msg "$GREEN" "🔨 Building OBSBOT Meet 2 Control..."

    # Create build directory
    if [ ! -d "$BUILD_DIR" ]; then
        print_msg "$BLUE" "Creating build directory..."
        mkdir -p "$BUILD_DIR"
    fi

    cd "$BUILD_DIR"

    print_msg "$BLUE" "Running CMake..."
    cmake ..

    print_msg "$BLUE" "Compiling with $(nproc) cores..."
    make -j$(nproc)

    cd ..

    print_msg "$GREEN" "✓ Build complete!"
    print_msg "$NC" "\nBinaries are in:"
    print_msg "$BLUE" "  - $BUILD_DIR/obsbot-meet2-gui (GUI application)"
    print_msg "$BLUE" "  - $BUILD_DIR/obsbot-meet2-cli (CLI tool)"
}

# Install the project
do_install() {
    # Build first
    do_build

    print_msg "$GREEN" "\n📦 Installing to $INSTALL_DIR..."

    # Create install directory if it doesn't exist
    if [ ! -d "$INSTALL_DIR" ]; then
        print_msg "$YELLOW" "Install directory doesn't exist."
        print_msg "$BLUE" "Creating: $INSTALL_DIR"
        mkdir -p "$INSTALL_DIR"
    fi

    # Copy GUI binary (always installed)
    print_msg "$BLUE" "Installing GUI application..."
    cp "$BUILD_DIR/obsbot-meet2-gui" "$INSTALL_DIR/"
    chmod +x "$INSTALL_DIR/obsbot-meet2-gui"

    local installed_apps="  - obsbot-meet2-gui"

    # Ask about CLI installation
    echo ""
    read -p "Install CLI tool as well? [y/N] " -n 1 -r
    echo

    if [[ $REPLY =~ ^[Yy]$ ]]; then
        print_msg "$BLUE" "Installing CLI tool..."
        cp "$BUILD_DIR/obsbot-meet2-cli" "$INSTALL_DIR/"
        chmod +x "$INSTALL_DIR/obsbot-meet2-cli"
        installed_apps="$installed_apps\n  - obsbot-meet2-cli"
    else
        print_msg "$YELLOW" "Skipping CLI installation."
    fi

    # Install desktop launcher
    echo ""
    print_msg "$BLUE" "Installing desktop launcher..."
    mkdir -p "$DESKTOP_DIR"
    mkdir -p "$ICON_DIR"

    cp "obsbot-meet2-control.desktop" "$DESKTOP_DIR/"
    chmod +x "$DESKTOP_DIR/obsbot-meet2-control.desktop"

    cp "resources/icons/camera.svg" "$ICON_DIR/obsbot-meet2-control.svg"

    # Update desktop database if available
    if command -v update-desktop-database &> /dev/null; then
        update-desktop-database "$DESKTOP_DIR" 2>/dev/null || true
    fi

    print_msg "$GREEN" "\n✓ Installation complete!"
    print_msg "$NC" "\nInstalled applications:"
    echo -e "$BLUE$installed_apps$NC"
    print_msg "$NC" "\nDesktop launcher installed - check your application menu!"

    # Check if install dir is in PATH
    if ! check_path "$INSTALL_DIR"; then
        offer_path_update "$INSTALL_DIR"
    else
        print_msg "$GREEN" "\n✓ $INSTALL_DIR is already in your PATH"
        print_msg "$NC" "You can run the applications from anywhere:"
        print_msg "$BLUE" "  obsbot-meet2-gui"
    fi
}

# Clean build directory
do_clean() {
    if [ -d "$BUILD_DIR" ]; then
        print_msg "$YELLOW" "🧹 Removing build directory..."
        rm -rf "$BUILD_DIR"
        print_msg "$GREEN" "✓ Clean complete!"
    else
        print_msg "$YELLOW" "Build directory doesn't exist. Nothing to clean."
    fi
}

# Main script logic
main() {
    if [ $# -eq 0 ]; then
        show_usage
        exit 0
    fi

    local command=$1
    local confirm=false

    # Check for --confirm flag
    if [ $# -gt 1 ] && [ "$2" == "--confirm" ]; then
        confirm=true
    fi

    case "$command" in
        build)
            if [ "$confirm" = false ]; then
                print_msg "$YELLOW" "⚠️  DRY RUN MODE - No changes will be made"
                echo ""
                print_msg "$NC" "This will:"
                print_msg "$BLUE" "  1. Create build/ directory"
                print_msg "$BLUE" "  2. Run CMake to configure the project"
                print_msg "$BLUE" "  3. Compile GUI and CLI applications"
                print_msg "$BLUE" "  4. Place binaries in build/"
                echo ""
                print_msg "$YELLOW" "To actually build, run:"
                print_msg "$GREEN" "  ./build.sh build --confirm"
                exit 0
            fi
            do_build
            ;;

        install)
            if [ "$confirm" = false ]; then
                print_msg "$YELLOW" "⚠️  DRY RUN MODE - No changes will be made"
                echo ""
                print_msg "$NC" "This will:"
                print_msg "$BLUE" "  1. Build the project (see: ./build.sh build)"
                print_msg "$BLUE" "  2. Create $INSTALL_DIR if needed"
                print_msg "$BLUE" "  3. Copy binaries to $INSTALL_DIR"
                print_msg "$BLUE" "  4. Make binaries executable"
                print_msg "$BLUE" "  5. Install desktop launcher to $DESKTOP_DIR"
                print_msg "$BLUE" "  6. Install icon to $ICON_DIR"
                print_msg "$BLUE" "  7. Check if $INSTALL_DIR is in PATH"
                print_msg "$BLUE" "  8. Offer to add to PATH if needed (with your approval)"
                echo ""
                print_msg "$YELLOW" "To actually install, run:"
                print_msg "$GREEN" "  ./build.sh install --confirm"
                exit 0
            fi
            do_install
            ;;

        clean)
            if [ "$confirm" = false ]; then
                print_msg "$YELLOW" "⚠️  DRY RUN MODE - No changes will be made"
                echo ""
                print_msg "$NC" "This will:"
                print_msg "$BLUE" "  1. Remove the entire build/ directory"
                echo ""
                print_msg "$YELLOW" "To actually clean, run:"
                print_msg "$GREEN" "  ./build.sh clean --confirm"
                exit 0
            fi
            do_clean
            ;;

        help|--help|-h)
            show_usage
            ;;

        *)
            print_msg "$RED" "❌ Unknown command: $command"
            echo ""
            show_usage
            exit 1
            ;;
    esac
}

main "$@"
