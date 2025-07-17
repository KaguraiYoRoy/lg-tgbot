cmake_minimum_required(VERSION 3.10.2)
project(lg-tgbot)

if(POLICY CMP0074)
    cmake_policy(SET CMP0074 NEW) # find_package() uses <PackageName>_ROOT variables
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

# libs
## threads
find_package(Threads REQUIRED)

## zlib
find_package(ZLIB REQUIRED)

## openssl
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

## boost
set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.65.1 COMPONENTS system REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR_RELEASE})
add_definitions(-DBOOST_BIND_GLOBAL_PLACEHOLDERS)

include_directories(include)
set(LIBTGBOT_SRC_LIST
    src/tgbot-cpp/Api.cpp
    src/tgbot-cpp/Bot.cpp
    src/tgbot-cpp/EventHandler.cpp
    src/tgbot-cpp/TgException.cpp
    src/tgbot-cpp/TgTypeParser.cpp
    src/tgbot-cpp/net/BoostHttpOnlySslClient.cpp
    src/tgbot-cpp/net/CurlHttpClient.cpp
    src/tgbot-cpp/net/HttpParser.cpp
    src/tgbot-cpp/net/TgLongPoll.cpp
    src/tgbot-cpp/net/Url.cpp
    src/tgbot-cpp/tools/FileTools.cpp
    src/tgbot-cpp/tools/StringTools.cpp
    src/tgbot-cpp/types/BotCommandScope.cpp
    src/tgbot-cpp/types/ChatBoostSource.cpp
    src/tgbot-cpp/types/ChatMember.cpp
    src/tgbot-cpp/types/InlineQueryResult.cpp
    src/tgbot-cpp/types/InputFile.cpp
    src/tgbot-cpp/types/InputMedia.cpp 
    src/tgbot-cpp/types/InputMessageContent.cpp
    src/tgbot-cpp/types/MenuButton.cpp
    src/tgbot-cpp/types/MessageOrigin.cpp
    src/tgbot-cpp/types/PassportElementError.cpp
    src/tgbot-cpp/types/ReactionType.cpp)

set(LIBJSONCPP_SRC_LIST
    src/json/json_tool.h
    src/json/json_reader.cpp
    src/json/json_valueiterator.inl
    src/json/json_value.cpp
    src/json/json_writer.cpp
)

add_compile_options(-Wsign-compare)

add_library(tgbot ${LIBTGBOT_SRC_LIST})
add_library(jsoncpp ${LIBJSONCPP_SRC_LIST})

add_executable(lg-tgbot src/lg-tgbot.cpp)
add_executable(lg-tgbot-agent src/agent.cpp)

target_link_libraries(lg-tgbot
    tgbot
    jsoncpp
    ${CMAKE_THREAD_LIBS_INIT}
    ${ZLIB_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    ${Boost_LIBRARIES}
)

target_link_libraries(lg-tgbot-agent
    jsoncpp
)

# target_include_directories(${PROJECT_NAME} PUBLIC 
#                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#                            $<INSTALL_INTERFACE:include>)
# target_link_libraries(${PROJECT_NAME} ${LIB_LIST})
# include(GNUInstallDirs)
# install(TARGETS ${PROJECT_NAME}
#         EXPORT ${PROJECT_NAME}-targets
#         RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
#         LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#         ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
# install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
# install(EXPORT ${PROJECT_NAME}-targets
#         NAMESPACE ${PROJECT_NAME}::
#         FILE ${PROJECT_NAME}Config.cmake
#         DESTINATION lib/cmake/${PROJECT_NAME})
# set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
