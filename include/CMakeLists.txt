# Creates a library iris which is an interface (header files only)
add_library(iris INTERFACE)
# If you want to use iris prefer to link against iris using this alias target
add_library(iris::iris ALIAS iris)
# Set C++ version
target_compile_features(iris INTERFACE cxx_std_20)
set_target_properties(iris PROPERTIES
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
)

# Add include folders to the library and targets that consume it the SYSTEM
# keyword suppresses warnings for users of the library
if(IRIS_STANDALONE_PROJECT)
  target_include_directories(iris INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )
else()
  target_include_directories(iris SYSTEM INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )
endif()

# Add (header only) external libraries
# find_package(Boost 1.61.0 REQUIRED)
# target_link_libraries(iris SYSTEM INTERFACE Boost::boost)
