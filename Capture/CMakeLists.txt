file(GLOB_RECURSE ALL_SRC *.cpp)
message(STATUS "${ALL_SRC}")
add_library(Capture STATIC ${ALL_SRC})

target_include_directories(Capture
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}  
        ${PROJECT_ROOT}/CHATCBBCommon/
        ${PROJECT_ROOT}/CHATCBBThirdPartyDepends/include 
        ${PROJECT_ROOT}/CHATCBBThirdPartyDepends/include/ffmpeg_static 
)
target_link_libraries(Capture
    PRIVATE
        LockFreeMPSCLogger
        avdevice
        avformat
        avcodec
        avutil
        swscale
        swresample
        avfilter
        pthread 
)
