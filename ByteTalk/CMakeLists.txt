file(GLOB_RECURSE MAIN_SRC "*.cpp")

add_executable(ByteTalk ${MAIN_SRC})

target_link_libraries(ByteTalk
    PRIVATE
        pthread
        ModuleController
        LockFreeMPSCLogger
        CHATCBBThirdPartyDepends
        Capture
)

target_include_directories(ByteTalk
    PRIVATE
        ${PROJECT_ROOT}/CHATProcessor
        ${PROJECT_ROOT}/CHATCBBCommon
        ${PROJECT_ROOT}/
        ${PROJECT_ROOT}/third_party/ffmpeg/build/include
)