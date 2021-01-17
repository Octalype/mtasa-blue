#
# Server lua library target
#
if (NOT MTASA_OS_WINDOWS)
    add_library(mtasa-server-lua ALIAS vendor-lua)
    return()
endif()

add_library(mtasa-server-lua SHARED $<TARGET_PROPERTY:vendor-lua,SOURCES>)

target_include_directories(mtasa-server-lua PUBLIC
    $<TARGET_PROPERTY:vendor-lua,INCLUDE_DIRECTORIES>
)

target_compile_definitions(mtasa-server-lua PUBLIC LUA_BUILD_AS_DLL)

set_target_properties(mtasa-server-lua PROPERTIES OUTPUT_NAME "lua5.1")

mtasa_set_target_outputdir(mtasa-server-lua "${MTASA_SERVER_MOD_OUTPUT_DIR}")
