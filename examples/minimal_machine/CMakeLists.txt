add_executable(minimal_machine main.cpp)
target_link_libraries(minimal_machine PRIVATE OpenMachineLab Threads::Threads)
# Statically link the mingw runtime (libgcc / libstdc++ / winpthread) so the
# executable runs on a plain Windows console without the ucrt64 DLLs on PATH.
target_link_options(minimal_machine PRIVATE -static)
