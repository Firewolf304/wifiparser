# Aboud
    Simple wifi parser

# Install

    sudo pacman -S iwlib libnm wireless_tools
    git clone https://github.com/Firewolf304/wifiparser.git
    cd wifiparser

# Build

    mkdir build && cd build
    cmake -G Ninja .. && ninja -j $(nproc)