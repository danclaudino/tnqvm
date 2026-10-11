set (LIBRARY_NAME tnqvm-itensor)

find_package(BLAS REQUIRED)
find_package(LAPACK REQUIRED)

find_program(JULIA_EXECUTABLE julia REQUIRED)
# Get Julia's binary directory
execute_process(
    COMMAND ${JULIA_EXECUTABLE} -e "using Libdl; println(Sys.BINDIR)"
    OUTPUT_VARIABLE JULIA_BINDIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Derive Julia root (parent of bin/)
get_filename_component(JULIA_ROOT ${JULIA_BINDIR} DIRECTORY)

# Construct include and lib paths
set(JULIA_INCLUDE_DIR "${JULIA_ROOT}/include/julia")
set(JULIA_LIBRARY_DIR "${JULIA_ROOT}/lib")

# Optionally report
message(STATUS "Found Julia include: ${JULIA_INCLUDE_DIR}")
message(STATUS "Found Julia library: ${JULIA_LIBRARY_DIR}")

file (GLOB HEADERS *.h *.hpp mps/*.hpp)
set (SRC ITensorMPSVisitor.cpp ITensorActivator.cpp)

usFunctionGetResourceSource(TARGET ${LIBRARY_NAME} OUT SRC)
usFunctionGenerateBundleInit(TARGET ${LIBRARY_NAME} OUT SRC)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-result -O2 -DNDEBUG -DPLATFORM_lapack -D__ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES=0")
add_library(${LIBRARY_NAME} SHARED ${SRC})

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
  target_compile_definitions(${LIBRARY_NAME} PUBLIC ITENSOR_USE_OMP)
  target_link_libraries(${LIBRARY_NAME} PUBLIC OpenMP::OpenMP_CXX)
endif()

set(_bundle_name tnqvm_itensor)
set_target_properties(${LIBRARY_NAME} PROPERTIES
    # This is required for every bundle
    COMPILE_DEFINITIONS US_BUNDLE_NAME=${_bundle_name}
    # This is for convenience, used by other CMake functions
    US_BUNDLE_NAME ${_bundle_name}
    )

# Embed meta-data from a manifest.json file
usFunctionEmbedResources(TARGET ${LIBRARY_NAME}
    WORKING_DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}
    FILES
    manifest.json
    )

find_library(JULIA_LIB julia REQUIRED PATHS ${JULIA_LIBRARY_DIR})
find_library(JULIA_BINDINGS juliaitensortnqvm HINTS ${JULIA_BINDINGS_PATH} REQUIRED)

target_include_directories(${LIBRARY_NAME} PUBLIC ${XACC_INCLUDE_ROOT}/eigen ${JULIA_INCLUDE_DIR})
target_link_libraries(${LIBRARY_NAME} PUBLIC xacc::xacc xacc::quantum_gate ${BLAS_LIBRARIES} ${LAPACK_LIBRARIES} ${JULIA_LIB} ${JULIA_BINDINGS})

xacc_configure_plugin_rpath(${LIBRARY_NAME})

install(TARGETS ${LIBRARY_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/plugins)

if(TNQVM_BUILD_TESTS)
	add_subdirectory(tests)
endif()

