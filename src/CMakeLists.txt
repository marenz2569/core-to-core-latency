# core-to-core-latency executable
add_executable(core-to-core-latency
    core-to-core-latency/CachelineToChaMapper.cpp
    core-to-core-latency/ChaToCoreMapper.cpp
    core-to-core-latency/Config.cpp
    core-to-core-latency/CoreTrafficTest.cpp
    core-to-core-latency/Main.cpp
    core-to-core-latency/TestList.cpp
    )

target_link_libraries(core-to-core-latency
    Threads::Threads
    firestartercore
    asmjit::asmjit
    PCM_STATIC
    )