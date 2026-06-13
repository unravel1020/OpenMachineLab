add_executable(file_driven_device main.cpp)
target_link_libraries(file_driven_device PRIVATE OpenMachineLab Threads::Threads)
# Standalone (no ucrt64 DLLs needed at runtime).
target_link_options(file_driven_device PRIVATE -static)

# Copy the data files next to the executable so it finds them at runtime.
configure_file(diebonder.cfg   diebonder.cfg   COPYONLY)
configure_file(diebonder.recipe diebonder.recipe COPYONLY)
