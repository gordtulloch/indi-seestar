#!/bin/bash
# Test script for INDI Alpaca Drivers
# Starts indiserver with all Alpaca drivers

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${BLUE}=====================================${NC}"
echo -e "${BLUE}INDI Alpaca Drivers Test Script${NC}"
echo -e "${BLUE}=====================================${NC}"
echo ""

# Check if drivers are built
BUILD_DIR="./build"
DRIVERS=(
    "indi_alpaca_telescope"
    "indi_alpaca_ccd"
    "indi_alpaca_filterwheel"
    "indi_alpaca_focuser"
)

echo -e "${YELLOW}Checking for built drivers...${NC}"
MISSING_DRIVERS=()
for driver in "${DRIVERS[@]}"; do
    if [ ! -f "$BUILD_DIR/$driver" ]; then
        MISSING_DRIVERS+=("$driver")
    else
        echo -e "${GREEN}✓${NC} $driver"
    fi
done

if [ ${#MISSING_DRIVERS[@]} -gt 0 ]; then
    echo -e "\n${RED}Error: Missing drivers:${NC}"
    for driver in "${MISSING_DRIVERS[@]}"; do
        echo -e "  ${RED}✗${NC} $driver"
    done
    echo -e "\n${YELLOW}Please build the drivers first:${NC}"
    echo "  cd build && cmake .. && make"
    exit 1
fi

# Copy XML file to INDI share directory
echo ""
echo -e "${YELLOW}Installing driver XML file...${NC}"
XML_SOURCE="./indi-alpaca.xml"
XML_DEST="/usr/share/indi/indi-alpaca.xml"
if [ -f "$XML_SOURCE" ]; then
    sudo cp "$XML_SOURCE" "$XML_DEST" 2>/dev/null && \
        echo -e "${GREEN}✓${NC} Copied alpaca.xml to $XML_DEST" || \
        echo -e "${YELLOW}⚠${NC} Could not copy XML file (may need sudo)"
else
    echo -e "${YELLOW}⚠${NC} XML file not found at $XML_SOURCE"
fi

echo ""
echo -e "${YELLOW}Available driver options:${NC}"
echo "  1) All drivers (telescope, ccd, filterwheel, focuser, dome)"
echo "  2) Telescope only"
echo "  3) CCD only"
echo "  4) Telescope + CCD + Focuser"
echo "  5) Custom selection"
echo "  6) Verbose mode (all drivers with debug output)"
echo ""
read -p "Select option [1-6] (default: 1): " option
option=${option:-1}

# Build the driver list
DRIVER_LIST=""
VERBOSE=""

case $option in
    1)
        DRIVER_LIST="$BUILD_DIR/indi_alpaca_telescope $BUILD_DIR/indi_alpaca_ccd $BUILD_DIR/indi_alpaca_filterwheel $BUILD_DIR/indi_alpaca_focuser $BUILD_DIR/indi_alpaca_dome"
        echo -e "${GREEN}Starting all Alpaca drivers...${NC}"
        ;;
    2)
        DRIVER_LIST="$BUILD_DIR/indi_alpaca_telescope"
        echo -e "${GREEN}Starting telescope driver...${NC}"
        ;;
    3)
        DRIVER_LIST="$BUILD_DIR/indi_alpaca_ccd"
        echo -e "${GREEN}Starting CCD driver...${NC}"
        ;;
    4)
        DRIVER_LIST="$BUILD_DIR/indi_alpaca_telescope $BUILD_DIR/indi_alpaca_ccd $BUILD_DIR/indi_alpaca_focuser"
        echo -e "${GREEN}Starting telescope, CCD, and focuser drivers...${NC}"
        ;;
    5)
        echo ""
        echo "Select drivers (y/n for each):"
        read -p "  Telescope? [y/n]: " sel_tele
        read -p "  CCD? [y/n]: " sel_ccd
        read -p "  FilterWheel? [y/n]: " sel_fw
        read -p "  Focuser? [y/n]: " sel_foc
        read -p "  Dome? [y/n]: " sel_dome
        
        [ "$sel_tele" = "y" ] && DRIVER_LIST="$DRIVER_LIST $BUILD_DIR/indi_alpaca_telescope"
        [ "$sel_ccd" = "y" ] && DRIVER_LIST="$DRIVER_LIST $BUILD_DIR/indi_alpaca_ccd"
        [ "$sel_fw" = "y" ] && DRIVER_LIST="$DRIVER_LIST $BUILD_DIR/indi_alpaca_filterwheel"
        [ "$sel_foc" = "y" ] && DRIVER_LIST="$DRIVER_LIST $BUILD_DIR/indi_alpaca_focuser"
        [ "$sel_dome" = "y" ] && DRIVER_LIST="$DRIVER_LIST $BUILD_DIR/indi_alpaca_dome"
        
        echo -e "${GREEN}Starting selected drivers...${NC}"
        ;;
    6)
        DRIVER_LIST="$BUILD_DIR/indi_alpaca_telescope $BUILD_DIR/indi_alpaca_ccd $BUILD_DIR/indi_alpaca_filterwheel $BUILD_DIR/indi_alpaca_focuser $BUILD_DIR/indi_alpaca_dome"
        VERBOSE="-v"
        echo -e "${GREEN}Starting all drivers in verbose mode...${NC}"
        ;;
    *)
        echo -e "${RED}Invalid option${NC}"
        exit 1
        ;;
esac

if [ -z "$DRIVER_LIST" ]; then
    echo -e "${RED}No drivers selected!${NC}"
    exit 1
fi

echo ""
echo -e "${YELLOW}Configuration:${NC}"
echo "  Server: localhost:7624"
echo "  Drivers: $DRIVER_LIST"
echo ""
echo -e "${YELLOW}Connection info:${NC}"
echo "  - Make sure your Alpaca device is accessible on the network"
echo "  - Default Alpaca hostname: alpaca.local"
echo "  - Default Alpaca port: 32323"
echo "  - Configure in INDI client connection properties"
echo ""
echo -e "${YELLOW}Testing tools:${NC}"
echo "  indi_getprop \"Alpaca Telescope.*\"  # List telescope properties"
echo "  indi_getprop \"Alpaca CCD.*\"        # List CCD properties"
echo "  indi_eval                            # Interactive INDI client"
echo ""
echo -e "${YELLOW}Press Ctrl+C to stop the server${NC}"
echo -e "${BLUE}=====================================${NC}"
echo ""

# Kill any existing indiserver instances
pkill -9 indiserver 2>/dev/null
sleep 1

# Start indiserver
if [ -n "$VERBOSE" ]; then
    exec indiserver $VERBOSE $DRIVER_LIST
else
    exec indiserver $DRIVER_LIST
fi
