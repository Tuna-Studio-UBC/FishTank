set -e 

# Clean
if [[ "$1" == "clean" ]]; then
    rm -rf build

# Build
elif [[ "$1" == "build" ]]; then
    shift
    # Full rebuild
    if [[ $1 =~ ^-(-?full|f)$ ]]; then
        rm -rf build
        cmake -B build
    fi
    cmake --build build

# Run
elif [[ "$1" == "run" ]]; then
    shift
    # Full rebuild
    if [[ $1 =~ ^-(-?full|f)$ ]]; then
        rm -rf build
        cmake -B build
        cmake --build build
    elif [[ $1 =~ ^-(-?build|b)$ ]]; then
        cmake --build build
    elif [[ $1 =~ ^-(-?tracy|t)$ ]]; then
        if [[ "$OSTYPE" == "darwin"* ]]; then
            ./profiler/tracy-profiler &
        elif [[ "$OSTYPE" == "msys"* ]]; then
            ./profiler/tracy-profiler.exe &
        fi
    fi

    # Visual Studio puts the exe in the build's respective Configuration directory
    ./build/fishtank || ./build/Debug/fishtank

# DevOps
elif [[ "$1" == "devops" ]]; then
    shift
    # Install dependencies
    if [[ $1 =~ ^-(-?install|i|-?all|a)$ ]]; then
        # Mac
        if [[ "$OSTYPE" == "darwin"* ]]; then
            if ![[ python --version &> /dev/null ]]; then
                brew install python
            fi
            if ![[ clang-format --version &> /dev/null ]]; then
                brew install clang-format
            fi
        
        # Windows (Git Bash)
        elif [[ "$OSTYPE" == "msys"* ]]; then
            if ![[ python --version &> /dev/null ]]; then
                winget install --id Python.Python.3.14 -e
            fi
            if ![[ clang-format --version &> /dev/null ]]; then
                winget install --id LLVM.ClangFormat -e
            fi
        fi

        # Install pre-commit
        pip install pre-commit
    fi

    # Test run pre-commit
    if [[ $1 =~ ^-(-?pre-commit|p|-?all|a)$ ]]; then
        pre-commit install-hooks
        pre-commit run --all-files
    fi

# Help
else
    printf '\nUsage: \n'
    printf '  ./run.sh <command> [options]\n'

    printf '\nCommands: \n'
    printf '  clean \t\t\t Delete all builds.\n'
    printf '  build \t\t\t Create a build.\n'
    printf '  run \t\t\t\t Run FishTank!\n'
    printf '  devops \t\t\t Perform DevOps tasks.\n'
    
    printf '\nOptions: \n'
    printf '  clean: \n'
    printf '    <none>\t\t\t Removes the build directory.\n'
    printf '  build: \n'
    printf '    <none>\t\t\t Perform a normal build.\n'
    printf '    -f, --full\t\t\t Perform a full build.\n'
    printf '  run: \n'
    printf '    <none>\t\t\t Run FishTank!\n'
    printf '    -b, --build\t\t\t Build and run FishTank!\n'
    printf '    -f, --full\t\t\t Full build and run FishTank!\n'
    printf '  devops: \n'
    printf '    -a, --all \t\t\t Perform all options.\n'
    printf '    -i, --install \t\t Install DevOps dependencies.\n'
    printf '    -p, --pre-commit \t\t Run pre-commit hook.\n'
fi
