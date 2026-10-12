add_library(acme_math STATIC src/math.cpp)
target_include_directories(acme_math PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_definitions(acme_math PRIVATE ACME_APP_LABEL="${ACME_APP_LABEL}")
