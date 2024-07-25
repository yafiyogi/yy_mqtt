include(CMakeFindDependencyMacro)
find_dependency(yy_cpp 0.0.1)
find_dependency(yy_json 0.0.1)
include(${CMAKE_CURRENT_LIST_DIR}/yy_mqttTargets.cmake)