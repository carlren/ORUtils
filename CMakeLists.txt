##############################
# CMakeLists.txt for ORUtils #
##############################

###########################
# Specify the target name #
###########################

SET(targetname ORUtils)

################################
# Specify the libraries to use #
################################

INCLUDE(${PROJECT_SOURCE_DIR}/cmake/UseCUDA.cmake)
INCLUDE(${PROJECT_SOURCE_DIR}/cmake/UseOpenMP.cmake)
INCLUDE(${PROJECT_SOURCE_DIR}/cmake/UsePNG.cmake)

#############################
# Specify the project files #
#############################
file(GLOB_RECURSE srcs *.cpp)
file(GLOB_RECURSE header *.hpp *.h)
file(GLOB_RECURSE cuda *.cu)


SET(sources
${srcs} ${cuda}
)
SET(headers
${header}
)

#############################
# Specify the source groups #in
#############################

SOURCE_GROUP("" FILES ${sources} ${headers})

##########################################
# Specify the target and where to put it #
##########################################

INCLUDE(${PROJECT_SOURCE_DIR}/cmake/SetCUDALibTarget.cmake)

set_target_properties(${targetname} PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(${targetname} PROPERTIES CXX_STANDARD 11)
#target_include_directories(${targetname} PUBLIC ./include PUBLIC .)
