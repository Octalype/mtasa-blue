#
# Server PCRE library target
#
if (NOT MTASA_OS_WINDOWS)
    add_library(mtasa-server-pcre ALIAS vendor-pcre)
    return()
endif()

set(VENDOR_PCRE_DIR "${MTASA_VENDOR_DIR}/pcre")

add_library(mtasa-server-pcre SHARED $<TARGET_PROPERTY:vendor-pcre,SOURCES>)

target_include_directories(mtasa-server-pcre PUBLIC "${VENDOR_PCRE_DIR}")

target_compile_definitions(mtasa-server-pcre
    PRIVATE
        HAVE_STRTOLL

    PUBLIC
        HAVE_CONFIG_H
)

set_target_properties(mtasa-server-pcre PROPERTIES OUTPUT_NAME "pcre3")

mtasa_set_target_outputdir(mtasa-server-pcre "${MTASA_SERVER_MOD_OUTPUT_DIR}")
