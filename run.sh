set -e 

# Run
if [[ "$1" == "run" ]]; then
    shift
    # Run with full rebuid
    if [[ $* == -+(-full|f) ]]; then
        rm -rf build
        cmake -B build
    fi

    # Visual Studio puts the exe in the build's respective Configuration directory
    cmake --build build
    ./build/fishtank || ./build/Debug/fishtank

# DevOps
elif [[ "$1" == "devops" ]]; then
    shift
    # Install dependencies
    if [[ $* == -+(-install|i|-all|a) ]]; then
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
    if [[ $* == -+(-pre-commit|p|-all|a) ]]; then
        pre-commit install
        pre-commit run --all-files
    fi

# Help
else
    printf '\nUsage: \n'
    printf '  ./run.sh <command> [options]\n'

    printf '\nCommands: \n'
    printf '  run \t\t\t\t Start the program.\n'
    printf '  devops \t\t\t Perform DevOps tasks.\n'
    
    printf '\nOptions:'
    printf '\n  run: \n'
    printf '    -f, --full\t\t\t Start the program w/ a full rebuild.\n'
    printf '\n  devops: \n'
    printf '    -a, --all \t\t\t Perform all options.\n'
    printf '    -i, --install \t\t Install dependencies.\n'
    printf '    -p, --pre-commit \t\t Run pre-commit hook.\n'
fi
