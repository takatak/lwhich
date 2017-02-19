cmake_minimum_required(VERSION 2.8)

# Configure CCache if available
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)


add_definitions("-Wall -Os -march=native") 
add_executable(lwhich
  lwhich.c
)
target_link_libraries(lwhich)
INSTALL(TARGETS lwhich RUNTIME DESTINATION bin)
