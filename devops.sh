set -e

# Install dependencies
if [[ "$1" == "--install" || "$1" == "-i" ]]; then
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
pre-commit install
pre-commit run --all-files
