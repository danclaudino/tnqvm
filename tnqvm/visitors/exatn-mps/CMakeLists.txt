find_package(ExaTN QUIET)

if (ExaTN_FOUND)
   message(STATUS "Found ExaTN at ${EXATN_ROOT}")
   set (PACKAGE_NAME "TNQVM ExaTN MPS Visitor")
   set (PACKAGE_DESCIPTION "TNQVM ExaTN MPS backend")
   set (LIBRARY_NAME tnqvm-exatn-mps)

   # Checks that EXATN_DIR is a valid ExaTN install directory

   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTNQVM_HAS_EXATN")
   set(CMAKE_INSTALL_RPATH "${EXATN_ROOT}/lib")
   set(EXATN_VISITOR_CPP_FILE ExaTnMpsVisitor.cpp)

   if (EXATN_BLAS_LIB MATCHES MKL)
      # Fix for bug #30
      message(STATUS "Exatn built with MKL, updating our build: ${EXATN_MKL_PATH}")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTNQVM_EXATN_USES_MKL_BLAS")
      configure_file("${CMAKE_CURRENT_SOURCE_DIR}/ExaTnMpsVisitor.cpp"
               "${CMAKE_BINARY_DIR}/tnqvm/visitors/exatn/ExaTnMpsVisitor.cpp" @ONLY)
      set(EXATN_VISITOR_CPP_FILE ${CMAKE_BINARY_DIR}/tnqvm/visitors/exatn/ExaTnMpsVisitor.cpp)
   endif()

   file (GLOB HEADERS *.hpp)
   file (GLOB SRC ${EXATN_VISITOR_CPP_FILE} ExaTnMpsActivator.cpp ExatnUtils.cpp)

   usFunctionGetResourceSource(TARGET ${LIBRARY_NAME} OUT SRC)
   usFunctionGenerateBundleInit(TARGET ${LIBRARY_NAME} OUT SRC)

   add_library(${LIBRARY_NAME} SHARED ${SRC})

   set(_bundle_name tnqvm_exatn_mps)
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

   target_include_directories(${LIBRARY_NAME} PUBLIC . ..)

   # Links to ExaTN using its linker config flags.
   target_link_libraries(${LIBRARY_NAME} PUBLIC xacc::xacc exatn::exatn xacc::quantum_gate)

   if(APPLE)
      set_target_properties(${LIBRARY_NAME} PROPERTIES INSTALL_RPATH "@loader_path/../lib")
      set_target_properties(${LIBRARY_NAME} PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
   else()
      set_target_properties(${LIBRARY_NAME} PROPERTIES INSTALL_RPATH "$ORIGIN/../lib;${EXATN_ROOT}/lib;${BLAS_PATH}")
      set_target_properties(${LIBRARY_NAME} PROPERTIES LINK_FLAGS "-shared")
   endif()

   install(TARGETS ${LIBRARY_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/plugins)

   if(TNQVM_BUILD_TESTS)
	   add_subdirectory(tests)
   endif()
else()
   message(STATUS "ExaTN not found, skipping ExaTNVisitor build")
endif()