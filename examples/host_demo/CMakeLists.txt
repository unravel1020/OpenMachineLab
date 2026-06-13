add_executable(host_demo main.cpp)
target_link_libraries(host_demo PRIVATE OpenMachineLab)
# Reach the device profiles living in the other example folders.
target_include_directories(host_demo PRIVATE ${CMAKE_SOURCE_DIR}/examples)
# Standalone (no ucrt64 DLLs needed at runtime).
target_link_options(host_demo PRIVATE -static)
