add_executable(pick_and_place main.cpp)
target_link_libraries(pick_and_place PRIVATE OpenMachineLab Threads::Threads)
# Standalone, like the other executables (no ucrt64 DLLs needed at runtime).
target_link_options(pick_and_place PRIVATE -static)
