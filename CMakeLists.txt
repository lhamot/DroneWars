cmake_minimum_required(VERSION 2.6)

# Nom du projet
project(DroneWars)
find_path(BOOST_INCLUDE_DIR boost
   PATH_SUFFIXES boost
   DOC "Include directory for boost"
   )
find_path(WT_INCLUDE_DIR Wt
   PATH_SUFFIXES Wt
   DOC "Include directory for Wt"
   )
find_path(LUABIND_INCLUDE_DIR luabind
   PATH_SUFFIXES luabind
   DOC "Include directory for luabind"
   )
find_path(LUA_INCLUDE_DIR lua
   PATH_SUFFIXES lua
   DOC "Include directory for lua"
   )
include_directories(${BOOST_INCLUDE_DIR})
include_directories(${WT_INCLUDE_DIR})
include_directories(${LUABIND_INCLUDE_DIR})
include_directories(${LUA_INCLUDE_DIR})

find_path(BOOST_LIBRARY NAMES boost
          DOC "The boost libraries path"
)
find_path(WT_LIBRARY NAMES wt
          DOC "The WT libraries path"
)
find_path(LUABIND_LIBRARY NAMES luabind
          DOC "The luabind libraries path"
)
find_path(LUA_LIBRARY NAMES luabind
          DOC "The lua libraries path"
)

link_directories(${BOOST_LIBRARY})
link_directories(${WT_LIBRARY}/src)
link_directories(${WT_LIBRARY}/src/Wt/Ext)
link_directories(${WT_LIBRARY}/src/http)
link_directories(${LUABIND_LIBRARY})
link_directories(${LUA_LIBRARY})

# D�claration de l'ex�cutable
add_executable(
    DroneWars
    src/bit_them_allWT.cpp
    src/Editor.cpp
    src/Engine.cpp       
    src/fighting.cpp
    src/FleetViewWT.cpp
    src/LuaTools.cpp
    src/LuaUniverse.cpp
    src/main.cpp
    src/MessageView.cpp
    src/Model.cpp
    src/OutPage.cpp
    src/PlanetViewWT.cpp
    src/Simulation.cpp
    src/stdafx.cpp
    src/stl_container_adapter.cpp
    src/TextGetter.cpp
    src/TranslationTools.cpp
    src/Rules.h
    src/serialize_unordered_map.h
    src/Tools.h
)

target_link_libraries( DroneWars debug wtd )
target_link_libraries( DroneWars debug wthttpd )
target_link_libraries( DroneWars debug wtextd )
target_link_libraries( DroneWars optimized wt )
target_link_libraries( DroneWars optimized wthttp )
target_link_libraries( DroneWars optimized wtext )

target_link_libraries( DroneWars debug luabindd )
target_link_libraries( DroneWars optimized luabind )

target_link_libraries( DroneWars debug luad )
target_link_libraries( DroneWars optimized lua )

if(CMAKE_COMPILER_IS_GNUCXX)
    target_link_libraries( DroneWars debug boost_threadd )
    target_link_libraries( DroneWars debug boost_signalsd )
    target_link_libraries( DroneWars debug boost_systemd )
    target_link_libraries( DroneWars debug boost_filesystemd )
    target_link_libraries( DroneWars debug boost_serializationd )
    target_link_libraries( DroneWars debug boost_iostreamsd )
    target_link_libraries( DroneWars optimized boost_thread )
    target_link_libraries( DroneWars optimized boost_signals )
    target_link_libraries( DroneWars optimized boost_system )
    target_link_libraries( DroneWars optimized boost_filesystem )
    target_link_libraries( DroneWars optimized boost_serialization )
    target_link_libraries( DroneWars optimized boost_iostreams )
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
endif()
if("${CMAKE_GENERATOR}" MATCHES "^Visual Studio")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd\"4996\" /wd\"4251\"")
endif()
              