cmake_minimum_required(VERSION 2.6)

# Nom du projet
project(DroneWars)
find_path(BOOST_INCLUDE_DIR boost
   PATH_SUFFIXES boost
   DOC "Include directory for boost"
   )
find_path(LUA_INCLUDE_DIR lua
   PATH_SUFFIXES lua
   DOC "Include directory for lua"
   )
find_path(THRIFT_INCLUDE_DIR thrift
   PATH_SUFFIXES thrift
   DOC "Include directory for thrift"
   )
find_path(POCO_INCLUDE_DIR poco
   PATH_SUFFIXES poco
   DOC "poco root directory for includes"
   )   
include_directories(${BOOST_INCLUDE_DIR})
include_directories(${LUA_INCLUDE_DIR})
include_directories(${THRIFT_INCLUDE_DIR})
include_directories(${POCO_INCLUDE_DIR}/Foundation/include)
include_directories(${POCO_INCLUDE_DIR}/Data/include)
include_directories(${POCO_INCLUDE_DIR}/Data/MySQL/include)

find_path(BOOST_LIBRARY NAMES boost
          DOC "The boost libraries path"
)
find_path(LUA_LIBRARY NAMES lua
          DOC "The lua libraries path"
)
find_path(THRIFT_LIBRARY NAMES thrift
          DOC "The thrift libraries path"
)
find_path(POCO_LIBRARY NAMES poco
          DOC "The POCO libraries path"
)

link_directories(${BOOST_LIBRARY})
link_directories(${LUA_LIBRARY})
link_directories(${THRIFT_LIBRARY})
link_directories(${POCO_LIBRARY})

# Déclaration de l'exécutable
add_executable(
    DroneWars
    src/DataBase.cpp
    src/Engine.cpp
    src/EngineServerHandler.cpp
    src/fighting.cpp
    src/LuaTools.cpp
    src/LuaUniverse.cpp
    src/main.cpp
    src/NameGen.cpp
    src/portable_binary_iarchive.cpp
    src/portable_binary_oarchive.cpp
    src/PTreeLuaHelper.cpp
    src/Rules.cpp
    src/Simulation.cpp
    src/Skills.cpp
    src/stdafx.cpp
    src/testFight.cpp
    src/Tools.cpp
    src/TypedPtree.cpp
    src/UnivManip.cpp
    src/Logger.cpp
    src/Model.cpp
    src/gen-cpp/EngineServer.cpp
    src/gen-cpp/thrift_constants.cpp
    src/gen-cpp/thrift_types.cpp
)


target_link_libraries( DroneWars debug luad5.2 )
target_link_libraries( DroneWars optimized lua5.2 )

target_link_libraries( DroneWars debug thriftd )
target_link_libraries( DroneWars optimized thrift )

target_link_libraries( DroneWars debug PocoDatad )
target_link_libraries( DroneWars debug PocoFoundationd )
target_link_libraries( DroneWars debug PocoDataMySQLd )

target_link_libraries( DroneWars optimized PocoData )
target_link_libraries( DroneWars optimized PocoFoundation )
target_link_libraries( DroneWars optimized PocoDataMySQL )

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
    target_link_libraries( DroneWars debug boost_logd )
	target_link_libraries( DroneWars debug boost_log_setupd )
    target_link_libraries( DroneWars optimized boost_thread )
    target_link_libraries( DroneWars optimized boost_signals )
    target_link_libraries( DroneWars optimized boost_system )
    target_link_libraries( DroneWars optimized boost_filesystem )
    target_link_libraries( DroneWars optimized boost_serialization )
    target_link_libraries( DroneWars optimized boost_iostreams )
    target_link_libraries( DroneWars optimized boost_chrono )
    target_link_libraries( DroneWars optimized boost_program_options )
    target_link_libraries( DroneWars optimized boost_locale )
    target_link_libraries( DroneWars optimized boost_log )
	target_link_libraries( DroneWars optimized boost_log_setup )
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")
	add_definitions(-DBOOST_LOG_DYN_LINK)
endif()
if("${CMAKE_GENERATOR}" MATCHES "^Visual Studio")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd\"4996\" /wd\"4251\"")
endif()
              
