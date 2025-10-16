#!/bin/bash
#
# OBSBOT Control - Quick Installer
#
# This script clones the repository and runs the automated build/install process.
# It will check dependencies and show what needs to be installed if anything is missing.
#

set -e

# Colors for output
if [ -t 1 ]; then
    RED='\033[0;31m'
    GREEN='\033[0;32m'
    YELLOW='\033[1;33m'
    BLUE='\033[0;34m'
    NC='\033[0m'
else
    RED=''
    GREEN=''
    YELLOW=''
    BLUE=''
    NC=''
fi

print_msg() {
    local color=$1
    shift
    echo -e "${color}$@${NC}"
}

print_msg "$GREEN" "═══════════════════════════════════════════════════════"
print_msg "$GREEN" "  OBSBOT Control - Quick Installer"
print_msg "$GREEN" "═══════════════════════════════════════════════════════"
echo ""

# Determine installation directory
INSTALL_BASE="${HOME}/src"
PROJECT_DIR="${INSTALL_BASE}/obsbot-controls-qt-linux"

print_msg "$BLUE" "📂 Installation directory: $PROJECT_DIR"
echo ""

# Check if git is installed
if ! command -v git &> /dev/null; then
    print_msg "$RED" "❌ Git is not installed"
    print_msg "$YELLOW" "Please install git first:"
    echo ""
    print_msg "$NC" "  Arch:         sudo pacman -S git"
    print_msg "$NC" "  Debian/Ubuntu: sudo apt install git"
    print_msg "$NC" "  Fedora:       sudo dnf install git"
    echo ""
    exit 1
fi

# Create base directory if needed
if [ ! -d "$INSTALL_BASE" ]; then
    print_msg "$YELLOW" "Creating directory: $INSTALL_BASE"
    mkdir -p "$INSTALL_BASE"
fi

# Clone or update repository
if [ -d "$PROJECT_DIR" ]; then
    print_msg "$YELLOW" "⚠️  Directory already exists: $PROJECT_DIR"
    echo ""
    read -p "Update existing installation? [Y/n] " -n 1 -r
    echo ""

    if [[ $REPLY =~ ^[Nn]$ ]]; then
        print_msg "$YELLOW" "Installation cancelled."
        exit 0
    fi

    print_msg "$BLUE" "📥 Updating repository..."
    cd "$PROJECT_DIR"
    git pull
else
    print_msg "$BLUE" "📥 Cloning repository..."
    git clone https://github.com/aaronsb/obsbot-controls-qt-linux.git "$PROJECT_DIR"
    cd "$PROJECT_DIR"
fi

echo ""
print_msg "$GREEN" "✓ Repository ready"
echo ""

# Run the build script
print_msg "$BLUE" "🔨 Starting build and installation..."
print_msg "$YELLOW" "Note: You'll be prompted to optionally install the CLI tool"
echo ""

# Give user a chance to cancel
sleep 2

# Run the build script with --confirm
if ./build.sh install --confirm; then
    echo ""
    print_msg "$GREEN" "═══════════════════════════════════════════════════════"
    print_msg "$GREEN" "  ✓ Installation Complete!"
    print_msg "$GREEN" "═══════════════════════════════════════════════════════"
    echo ""
    print_msg "$BLUE" "The application should now appear in your system menu."
    print_msg "$NC" "Or run from terminal:"
    print_msg "$GREEN" "  obsbot-gui"
    echo ""
    print_msg "$BLUE" "Repository location: $PROJECT_DIR"
    echo ""
else
    echo ""
    print_msg "$RED" "═══════════════════════════════════════════════════════"
    print_msg "$RED" "  ✗ Installation Failed"
    print_msg "$RED" "═══════════════════════════════════════════════════════"
    echo ""
    print_msg "$YELLOW" "The build script reported errors above."
    print_msg "$YELLOW" "Install any missing dependencies and try again:"
    echo ""
    print_msg "$GREEN" "  cd $PROJECT_DIR"
    print_msg "$GREEN" "  ./build.sh install --confirm"
    echo ""
    exit 1
fi
