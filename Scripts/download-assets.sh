#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
CYAN='\033[0;36m'
YELLOW='\033[1;33m'
NC='\033[0m'
BOLD='\033[1m'

FILES=(
    "default_texture.png"
    "fizzy.wav"
    "viking_room.zip"
    "robot.zip"
    "sponza.zip"
)

ASSETS_DIR="../.assets"

read -p "Enter the public R2 base URL: " BASE_URL

BASE_URL="${BASE_URL%/}"
BASE_URL=$(echo "$BASE_URL" | tr -d '\r')

if [ -z "$BASE_URL" ]; then
    echo -e "\n${RED}[!] Error: Base URL cannot be empty.${NC}"
    exit 1
fi

echo -e "\n${YELLOW}[i] Starting download of ${#FILES[@]} files...${NC}\n"

for FILE in "${FILES[@]}"; do
    FILE=$(echo "$FILE" | tr -d '\r')

    URL="$BASE_URL/$FILE"
    TARGET="$ASSETS_DIR/$FILE"

    echo -ne "${NC}Fetching: ${BOLD}$FILE${NC} ... "

    curl --create-dirs -o "$TARGET" -f -sS "$URL"

    if [ $? -eq 0 ]; then
        echo -e "${GREEN}SUCCESS${NC}"

        if [[ "$FILE" == *.zip ]]; then
            EXTRACT_DIR="$ASSETS_DIR/${FILE%.zip}"
            echo -ne "  ${CYAN}-> Extracting into ${EXTRACT_DIR}... ${NC}"

            unzip -q -o "$TARGET" -d "$EXTRACT_DIR"

            rm "$TARGET"
            echo -e "${GREEN}DONE & ZIP DELETED${NC}"
        fi
    else
        echo -e "${RED}FAILED${NC}"
        echo -e "  ${RED}-> Please check the curl error above or verify the file exists on R2.${NC}"
    fi
done
