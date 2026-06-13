# All tests share oml_test.h (TestBase). It is header-only and lives in this
# directory, so it is found via the same-dir include "".
set(OML_TESTS lifecycle_test perf_test safety_test)
foreach(t IN LISTS OML_TESTS)
    add_executable(${t} ${t}.cpp)
    target_link_libraries(${t} PRIVATE OpenMachineLab Threads::Threads)
    # Standalone, like the demo exe (no ucrt64 DLLs needed at runtime).
    target_link_options(${t} PRIVATE -static)
    add_test(NAME ${t} COMMAND ${t})
endforeach()
