cmake_minimum_required(VERSION 2.6)

# Nom du projet
project(DroneWars)
find_path(BOOST_INCLUDE_DIR boost
   PATH_SUFFIXES boost
   DOC "Include directory for boost"
   )
find_path(LUABIND_INCLUDE_DIR luabind
   PATH_SUFFIXES luabind
   DOC "Include directory for luabind"
   )
find_path(LUA_INCLUDE_DIR lua
   PATH_SUFFIXES lua
   DOC "Include directory for lua"
   )
find_path(THRIFT_INCLUDE_DIR thrift
   PATH_SUFFIXES thrift
   DOC "Include directory for thrift"
   )
find_path(LOG4CPLUS_INCLUDE_DIR log4cplus
   PATH_SUFFIXES log4cplus
   DOC "Include directory for log4cplus"
   )
include_directories(${BOOST_INCLUDE_DIR})
include_directories(${LUABIND_INCLUDE_DIR})
include_directories(${LUA_INCLUDE_DIR})
include_directories(${THRIFT_INCLUDE_DIR})
include_directories(${LOG4CPLUS_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/luabind)

find_path(BOOST_LIBRARY NAMES boost
          DOC "The boost libraries path"
)
find_path(LUABIND_LIBRARY NAMES luabind
          DOC "The luabind libraries path"
)
find_path(LUA_LIBRARY NAMES lua
          DOC "The lua libraries path"
)
find_path(THRIFT_LIBRARY NAMES thrift
          DOC "The thrift libraries path"
)
find_path(LOG4CPLUS_LIBRARY NAMES log4cplus
          DOC "The log4cplus libraries path"
)

link_directories(${BOOST_LIBRARY})
link_directories(${LUABIND_LIBRARY})
link_directories(${LUA_LIBRARY})
link_directories(${THRIFT_LIBRARY})
link_directories(${LOG4CPLUS_LIBRARY})

# Déclaration de l'exécutable
add_executable(
    DroneWars
    src/Engine.cpp       
    src/EngineServerHandler.cpp       
    src/fighting.cpp
    src/LuaTools.cpp
    src/LuaUniverse.cpp
    src/main.cpp
    src/Model.cpp
    src/NameGen.cpp
    src/Simulation.cpp
    src/stdafx.cpp
    src/stl_container_adapter.cpp
    src/Rules.cpp
    src/gen-cpp/EngineServer.cpp
    src/gen-cpp/thrift_constants.cpp
    src/gen-cpp/thrift_types.cpp
    src/portable_binary_iarchive.cpp
    src/portable_binary_oarchive.cpp
)

target_link_libraries( DroneWars debug luabindd )
target_link_libraries( DroneWars optimized luabind )

target_link_libraries( DroneWars debug luad )
target_link_libraries( DroneWars optimized lua )

target_link_libraries( DroneWars debug thriftd )
target_link_libraries( DroneWars optimized thrift )

target_link_libraries( DroneWars debug log4cplusd )
target_link_libraries( DroneWars optimized log4cplus )

if(CMAKE_COMPILER_IS_GNUCXX)
    target_link_libraries( DroneWars debug boost_threadd )
    target_link_libraries( DroneWars debug boost_signalsd )
    target_link_libraries( DroneWars debug boost_systemd )
    target_link_libraries( DroneWars debug boost_filesystemd )
    target_link_libraries( DroneWars debug boost_serializationd )
    target_link_libraries( DroneWars debug boost_iostreamsd )
    target_link_libraries( DroneWars debug boost_chronod )
    target_link_libraries( DroneWars debug boost_program_optionsd )
    target_link_libraries( DroneWars debug boost_localed )
    target_link_libraries( DroneWars optimized boost_thread )
    target_link_libraries( DroneWars optimized boost_signals )
    target_link_libraries( DroneWars optimized boost_system )
    target_link_libraries( DroneWars optimized boost_filesystem )
    target_link_libraries( DroneWars optimized boost_serialization )
    target_link_libraries( DroneWars optimized boost_iostreams )
    target_link_libraries( DroneWars optimized boost_chrono )
    target_link_libraries( DroneWars optimized boost_program_options )
    target_link_libraries( DroneWars optimized boost_locale )
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
endif()
if("${CMAKE_GENERATOR}" MATCHES "^Visual Studio")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd\"4996\" /wd\"4251\"")
endif()
              
