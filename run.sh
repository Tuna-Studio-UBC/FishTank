set -e 

if [[ "$1" == "remake" || "$1" == "r" ]]; then
    echo "Remake build"
    cmake --build build
    ./build/fishtank || ./build/Debug/fishtank
    exit 0
fi

echo "Full build"
rm -rf build
cmake -B build
cmake --build build

# Visual Studio puts the exe in the build's respective Configuration directory
./build/fishtank || ./build/Debug/fishtank
