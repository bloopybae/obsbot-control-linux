#!/bin/bash

# OBSBOT Meet 2 Control - Uninstall Script
# Safely removes installed files without touching system configuration

set -e

# Check if we're in an interactive terminal
if [ -t 1 ]; then
    # Colors for output
    RED='\033[0;31m'
    GREEN='\033[0;32m'
    YELLOW='\033[1;33m'
    BLUE='\033[0;34m'
    NC='\033[0m' # No Color
else
    # No colors for non-interactive output
    RED=''
    GREEN=''
    YELLOW=''
    BLUE=''
    NC=''
fi

# Configuration - must match build.sh
INSTALL_DIR="${XDG_BIN_HOME:-$HOME/.local/bin}"
DESKTOP_DIR="${XDG_DATA_HOME:-$HOME/.local/share}/applications"
ICON_DIR="${XDG_DATA_HOME:-$HOME/.local/share}/icons/hicolor/scalable/apps"

# Print colored message
print_msg() {
    local color=$1
    shift
    echo -e "${color}$@${NC}"
}

# Show usage information
show_usage() {
    echo -e "${GREEN}OBSBOT Meet 2 Control - Uninstall Script${NC}"
    echo -e ""
    echo -e "${YELLOW}Usage:${NC}"
    echo -e "  ./uninstall.sh [--confirm]"
    echo -e ""
    echo -e "${YELLOW}What it does:${NC}"
    echo -e "  Removes OBSBOT Meet 2 Control files from your system:"
    echo -e "  - ${BLUE}$INSTALL_DIR/obsbot-meet2-gui${NC}"
    echo -e "  - ${BLUE}$INSTALL_DIR/obsbot-meet2-cli${NC}"
    echo -e "  - ${BLUE}$DESKTOP_DIR/obsbot-meet2-control.desktop${NC}"
    echo -e "  - ${BLUE}$ICON_DIR/obsbot-meet2-control.svg${NC}"
    echo -e ""
    echo -e "${YELLOW}What it does NOT do:${NC}"
    echo -e "  - Does not modify shell configuration files (.bashrc, .zshrc)"
    echo -e "  - Does not remove PATH statements"
    echo -e "  - Does not remove configuration files (~/.config/obsbot-meet2-control/)"
    echo -e "  - Only removes files we explicitly installed"
    echo -e ""
    echo -e "${YELLOW}Options:${NC}"
    echo -e "  ${BLUE}--confirm${NC}"
    echo -e "    Required flag to actually uninstall. Without this flag,"
    echo -e "    the script will only show what it would do."
    echo -e ""
    echo -e "${YELLOW}Examples:${NC}"
    echo -e "  # See what will be removed (dry run)"
    echo -e "  ./uninstall.sh"
    echo -e ""
    echo -e "  # Actually remove installed files"
    echo -e "  ./uninstall.sh --confirm"
    echo -e ""
    echo -e "${YELLOW}Note:${NC}"
    echo -e "  If you want to remove configuration files as well, run:"
    echo -e "  ${BLUE}rm -rf ~/.config/obsbot-meet2-control/${NC}"
    echo -e ""
}

# Check if file exists and remove it
remove_file() {
    local file=$1
    local description=$2

    if [ -f "$file" ]; then
        rm -f "$file"
        print_msg "$GREEN" "✓ Removed: $description"
        return 0
    else
        print_msg "$YELLOW" "⚠️  Not found: $description"
        print_msg "$BLUE" "   ($file)"
        return 1
    fi
}

# Perform uninstallation
do_uninstall() {
    print_msg "$GREEN" "🗑️  Uninstalling OBSBOT Meet 2 Control..."
    echo ""

    local removed=0
    local not_found=0

    # Remove GUI binary
    if remove_file "$INSTALL_DIR/obsbot-meet2-gui" "GUI application"; then
        removed=$((removed + 1))
    else
        not_found=$((not_found + 1))
    fi

    # Remove CLI binary
    if remove_file "$INSTALL_DIR/obsbot-meet2-cli" "CLI tool"; then
        removed=$((removed + 1))
    else
        not_found=$((not_found + 1))
    fi

    # Remove desktop launcher
    if remove_file "$DESKTOP_DIR/obsbot-meet2-control.desktop" "Desktop launcher"; then
        removed=$((removed + 1))
    else
        not_found=$((not_found + 1))
    fi

    # Remove icon
    if remove_file "$ICON_DIR/obsbot-meet2-control.svg" "Application icon"; then
        removed=$((removed + 1))
    else
        not_found=$((not_found + 1))
    fi

    # Update desktop database if available
    if [ -f "$DESKTOP_DIR/obsbot-meet2-control.desktop" ] || command -v update-desktop-database &> /dev/null; then
        print_msg "$BLUE" "\nUpdating desktop database..."
        update-desktop-database "$DESKTOP_DIR" 2>/dev/null || true
    fi

    echo ""
    print_msg "$GREEN" "📊 Uninstall Summary:"
    print_msg "$GREEN" "   Files removed: $removed"
    if [ $not_found -gt 0 ]; then
        print_msg "$YELLOW" "   Files not found: $not_found"
    fi

    echo ""
    if [ $removed -gt 0 ]; then
        print_msg "$GREEN" "✓ Uninstall complete!"
    else
        print_msg "$YELLOW" "⚠️  No files were found to remove."
        print_msg "$BLUE" "   (OBSBOT Meet 2 Control may not have been installed)"
    fi

    echo ""
    print_msg "$BLUE" "Note: Configuration files preserved at:"
    print_msg "$BLUE" "  ~/.config/obsbot-meet2-control/"
    print_msg "$NC" "\nTo remove configuration, run:"
    print_msg "$YELLOW" "  rm -rf ~/.config/obsbot-meet2-control/"
    echo ""
    print_msg "$BLUE" "Note: Shell PATH modifications (if any) were NOT removed."
    print_msg "$NC" "If you added ~/.local/bin to your PATH, you may want to remove that line from:"
    print_msg "$YELLOW" "  ~/.bashrc or ~/.zshrc"
}

# Main script logic
main() {
    if [ $# -eq 0 ]; then
        # Dry run mode
        print_msg "$YELLOW" "⚠️  DRY RUN MODE - No changes will be made"
        echo ""
        print_msg "$NC" "This will check for and remove the following files:"
        echo ""

        # Check each file
        local total=0
        local found=0

        files=(
            "$INSTALL_DIR/obsbot-meet2-gui:GUI application"
            "$INSTALL_DIR/obsbot-meet2-cli:CLI tool"
            "$DESKTOP_DIR/obsbot-meet2-control.desktop:Desktop launcher"
            "$ICON_DIR/obsbot-meet2-control.svg:Application icon"
        )

        for entry in "${files[@]}"; do
            IFS=':' read -r file description <<< "$entry"
            total=$((total + 1))
            if [ -f "$file" ]; then
                print_msg "$GREEN" "  ✓ Found: $description"
                print_msg "$BLUE" "    $file"
                found=$((found + 1))
            else
                print_msg "$YELLOW" "  ✗ Not found: $description"
                print_msg "$BLUE" "    $file"
            fi
        done

        echo ""
        print_msg "$BLUE" "Files found: $found / $total"
        echo ""
        print_msg "$YELLOW" "To actually uninstall, run:"
        print_msg "$GREEN" "  ./uninstall.sh --confirm"
        echo ""
        print_msg "$BLUE" "Note: Configuration and PATH changes will NOT be removed."
        exit 0
    fi

    if [ "$1" == "--confirm" ]; then
        do_uninstall
    elif [ "$1" == "help" ] || [ "$1" == "--help" ] || [ "$1" == "-h" ]; then
        show_usage
    else
        print_msg "$RED" "❌ Unknown option: $1"
        echo ""
        show_usage
        exit 1
    fi
}

main "$@"
