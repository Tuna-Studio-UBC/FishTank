set -e 

# rm -rf build
# cmake -B build -DCMAKE_POLICY_VERSION_MINIMUM=3.5
cmake --build build
./build/tower_defense