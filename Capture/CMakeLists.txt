file(GLOB_RECURSE ALL_SRC *.cpp)
message(STATUS "${ALL_SRC}")
add_library(Capture STATIC ${ALL_SRC})

target_include_directories(Capture
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}  
        ${PROJECT_ROOT}/CHATCBBCommon/
        ${PROJECT_ROOT}/CHATCBBThirdPartyDepends/include 
        ${PROJECT_ROOT}/third_party/ffmpeg/build/include 
)
target_link_libraries(Capture
    PRIVATE
        LockFreeMPSCLogger
        ${PROJECT_ROOT}/third_party/ffmpeg/build/lib/libavdevice.a
        ${PROJECT_ROOT}/third_party/ffmpeg/build/lib/libavfilter.a
        ${PROJECT_ROOT}/third_party/ffmpeg/build/lib/libavformat.a
        ${PROJECT_ROOT}/third_party/ffmpeg/build/lib/libavcodec.a
        ${PROJECT_ROOT}/third_party/ffmpeg/build/lib/libswresample.a
        ${PROJECT_ROOT}/third_party/ffmpeg/build/lib/libswscale.a
        ${PROJECT_ROOT}/third_party/ffmpeg/build/lib/libavutil.a
        -lm
        -lpthread
        -ldl
)
