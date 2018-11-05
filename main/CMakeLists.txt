include_directories(${Caelum_SOURCE_DIR}/main/include/)
include_directories(${Caelum_BINARY_DIR}/main/include/)

FILE(GLOB sources ${Caelum_SOURCE_DIR}/main/src/*.cpp ${Caelum_SOURCE_DIR}/main/include/*.h)

set(LIBNAME "Caelum")

add_definitions("-DCAELUM_LIB")

ADD_LIBRARY(${LIBNAME} SHARED ${sources})

# install the library

INSTALL(TARGETS ${LIBNAME}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        )

if (INSTALL_OGRE_PLUGIN)
  INSTALL(TARGETS ${LIBNAME}
          RUNTIME DESTINATION bin
          LIBRARY DESTINATION ${LIB_INSTALL_DIR}
          ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
          )
endif (INSTALL_OGRE_PLUGIN)

# install the headers: both source and build headers
INSTALL(DIRECTORY ${Caelum_SOURCE_DIR}/main/include/ DESTINATION include/Caelum)


# --- Threading support (still needed for GCC even with C++11)
set(CMAKE_THREAD_PREFER_PTHREAD YES)
find_package(Threads REQUIRED)
target_link_libraries(${LIBNAME} PRIVATE
        Threads::Threads
        )

# --- Ogre 3D graphics engine ---
if(DEFINED CONAN_OGRE_ROOT)
      target_include_directories(${LIBNAME} PRIVATE CONAN_PKG::OGRE)
      target_link_libraries(${LIBNAME} PRIVATE CONAN_PKG::OGRE)
else()
      target_link_libraries(${LIBNAME} PRIVATE ${OGRE_LIBRARIES})
      target_include_directories(${LIBNAME} PRIVATE ${OGRE_INCLUDE_DIRS})
endif()
