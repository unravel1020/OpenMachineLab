add_executable(lifecycle_test lifecycle_test.cpp)
target_link_libraries(lifecycle_test PRIVATE OpenMachineLab Threads::Threads)
# Standalone, like the demo exe (no ucrt64 DLLs needed at runtime).
target_link_options(lifecycle_test PRIVATE -static)

add_test(NAME lifecycle_test COMMAND lifecycle_test)
