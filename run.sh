set -e 

rm -rf build
cmake -B build
cmake --build build

# Visual Studio puts the exe in the build's respective Configuration directory
./build/fishtank || ./build/Debug/fishtank
