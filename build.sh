#!/bin/bash
set -e

VERSION="v0.1.0"
ROOT_DIR=$(pwd)
BUILD_DIR="$ROOT_DIR/build"
CONTAINER_NAME="chat-dev"
DOCKER_IMAGE="ubuntu:24.04"
BUILD_TYPE=${1:-Release} 

virtualenv_build() {
    echo "===== Virtual Environment Build ====="

    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ..
    make -j$(nproc)
    if [ -d "$ROOT_DIR/configs" ]; then
        cp -r "$ROOT_DIR/configs" "$BUILD_DIR/"
    fi
    echo "Virtual build finished. Configs copied to $BUILD_DIR/configs"
}

docker_build() {
    echo "===== Docker Build ====="

    if docker ps -a --format '{{.Names}}' | grep -Eq "^${CONTAINER_NAME}\$"; then
        echo "Container $CONTAINER_NAME exists."
    else
        echo "Creating container $CONTAINER_NAME..."
        docker run -itd \
            --name "$CONTAINER_NAME" \
            -v "$ROOT_DIR":"$ROOT_DIR" \
            -w "$ROOT_DIR" \
            $DOCKER_IMAGE \
            bash
        docker exec "$CONTAINER_NAME" bash -c "
            apt-get update && apt-get install -y build-essential cmake git
            touch $ROOT_DIR/.deps_installed
        "
    fi

    docker exec -it "$CONTAINER_NAME" bash -c "
        cd $BUILD_DIR || mkdir -p $BUILD_DIR && cd $BUILD_DIR
        cmake $ROOT_DIR
        make -j\$(nproc)
        if [ -d $ROOT_DIR/configs ]; then cp -r $ROOT_DIR/configs $BUILD_DIR/; fi
        echo 'Docker build finished.'
    "
}

print_logo() {
    RED='\033[0;31m'
    GREEN='\033[0;32m'
    YELLOW='\033[1;33m'
    BLUE='\033[0;34m'
    MAGENTA='\033[0;35m'
    CYAN='\033[0;36m'
    BOLD='\033[1m'
    RESET='\033[0m'

    echo -e "${CYAN}------------------------------------------------------------${RESET}"
    echo -e "${BOLD}${CYAN}           ByteTalk ${VERSION}${RESET}"
    echo -e "${CYAN}------------------------------------------------------------${RESET}"
}



main() {
    print_logo;
    if [ "$1" == "--docker" ]; then
        docker_build
    else
        virtualenv_build
    fi
}

main "$1"
