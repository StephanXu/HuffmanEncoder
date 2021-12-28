cmake_minimum_required(VERSION 3.15)

project(HuffmanEncoder)

set(CMAKE_MFC_FLAG 1)

add_executable(HuffmanEncoder WIN32)
target_compile_definitions(HuffmanEncoder PRIVATE _AFXDLL)
target_precompile_headers(HuffmanEncoder PRIVATE "src/pch.h")
target_compile_features(HuffmanEncoder PRIVATE cxx_std_17)
target_sources(HuffmanEncoder
    PRIVATE
        "src/BitCollector.cpp"
        "src/FileDetailDlg.cpp"
        "src/Huffman.cpp"
        "src/Huffman.rc"
        "src/HuffmanDlg.cpp"
        "src/HuffmanEncoder.cpp"
        "src/MultilineList.cpp"
        "src/pch.cpp"
        "src/ProcessDlg.cpp"
        "src/Utils.cpp"
)


add_executable(UnitTest)

find_package(GTest CONFIG REQUIRED)

target_compile_features(UnitTest PRIVATE cxx_std_17)
target_compile_definitions(UnitTest PRIVATE _AFXDLL)
target_link_libraries(UnitTest PRIVATE GTest::gtest GTest::gtest_main)
target_sources(UnitTest 
    PRIVATE 
        "test/test.cpp"
        "src/HuffmanEncoder.cpp"
        "src/BitCollector.cpp"
)