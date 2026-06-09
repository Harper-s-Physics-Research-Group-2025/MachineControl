my_mathematica_project/
├── CMakeLists.txt          # Root CMake configuration
├── README.md               # Project documentation
├── .gitignore              # Git ignore file (ignores /build)
│
├── cmake/                  # Custom CMake scripts/modules (optional)
│   └── FindWolfram.cmake   # Optional script to auto-find Mathematica
│
├── include/                # Public header files (.h / .hpp)
│   └── my_project/         # Subfolder matching project name (prevents naming collisions)
│       ├── library_link.h  # Wrapper definitions for Wolfram
│       └── core_logic.h    # Shared business logic headers
│
├── src/                    # Source files (.cpp)
│   ├── library_link.cpp    # Entry point containing WolframLibrary_initialize, etc.
│   ├── algorithm_1.cpp     # Migrated executable 1 logic
│   ├── algorithm_2.cpp     # Migrated executable 2 logic
│   └── core_logic.cpp      # Shared helper code used by multiple algorithms
│
├── tests/                  # Unit tests (optional but recommended)
    ├── CMakeLists.txt
    └── test_main.cpp
    └── notebooks/              # Mathematica notebooks for testing the library
        └── test_interface.nb
