cmake_minimum_required(VERSION 3.21)

project(kimacchuNative VERSION 0.0.1 LANGUAGES CXX)

if(DEFINED ENV{SKYRIM_FOLDER} AND IS_DIRECTORY "$ENV{SKYRIM_FOLDER}/Data")
    set(OUTPUT_FOLDER "$ENV{SKYRIM_FOLDER}/Data")
endif()

if(DEFINED ENV{SKYRIM_MODS_FOLDER} AND IS_DIRECTORY "$ENV{SKYRIM_MODS_FOLDER}")
    set(OUTPUT_FOLDER "$ENV{SKYRIM_MODS_FOLDER}/${PROJECT_NAME}")
endif()

include_directories(./IWW/iWantWidgetsNG/include)

add_subdirectory(IWW/iWantWidgetsNG)

find_package(CommonLibSSE CONFIG REQUIRED)
find_package(Boost REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_commonlibsse_plugin(${PROJECT_NAME} SOURCES 
    src/KMCConfig.cpp
    src/KMCRegister.cpp
    src/KMCSound.cpp
    src/KMCPapyrus.cpp
    src/thread_pool.cpp
    src/KMCEventThread.cpp
    src/KMCUtility.cpp
    src/KMCStateManager.cpp
    src/KMCOAR.cpp
    src/KMCCutin.cpp
    src/KMCProfile.cpp
    src/KMCWaitTask.cpp
    src/KMCCutinCondition.cpp
    src/KMCCutinCond/KMCCJsonValueInterval.cpp
    src/KMCCutinCond/KMCChecks.cpp
    src/KMCCutinCond/KMCWorks.cpp
    src/KMCCutinCond/KMCSubTask.cpp
    src/KMCCutinCond/KMCCheckSource.cpp
    src/KMCGameEventListener.cpp
    src/KMCTempKeywordManager.cpp
    src/KMCExpression.cpp
    plugin.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC ./include)

target_link_libraries(${PROJECT_NAME} PUBLIC IWantWidgetsNative) 

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
target_precompile_headers(${PROJECT_NAME} PUBLIC PCH.h)

if(DEFINED OUTPUT_FOLDER)
    set(DLL_FOLDER "${OUTPUT_FOLDER}/SKSE/Plugins")

    message(STATUS "SKSE plugin output folder: ${DLL_FOLDER}")

    add_custom_command(
        TARGET "${PROJECT_NAME}"
        POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${DLL_FOLDER}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:${PROJECT_NAME}>" "${DLL_FOLDER}/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
        VERBATIM
    )

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_custom_command(
            TARGET "${PROJECT_NAME}"
            POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_PDB_FILE:${PROJECT_NAME}>" "${DLL_FOLDER}/$<TARGET_PDB_FILE_NAME:${PROJECT_NAME}>"
            VERBATIM
        )
    endif()
endif()
