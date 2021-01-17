#
# jpeg library target
#
if (NOT (MTASA_OS_WINDOWS AND MTASA_X86))
    return()
endif()

set(VENDOR_JPEG_DIR "${MTASA_VENDOR_DIR}/jpeg-9d")

add_library(vendor-jpeg STATIC
    "${VENDOR_JPEG_DIR}/jaricom.c"
    "${VENDOR_JPEG_DIR}/jcapimin.c"
    "${VENDOR_JPEG_DIR}/jcapistd.c"
    "${VENDOR_JPEG_DIR}/jcarith.c"
    "${VENDOR_JPEG_DIR}/jccoefct.c"
    "${VENDOR_JPEG_DIR}/jccolor.c"
    "${VENDOR_JPEG_DIR}/jcdctmgr.c"
    "${VENDOR_JPEG_DIR}/jchuff.c"
    "${VENDOR_JPEG_DIR}/jcinit.c"
    "${VENDOR_JPEG_DIR}/jcmainct.c"
    "${VENDOR_JPEG_DIR}/jcmarker.c"
    "${VENDOR_JPEG_DIR}/jcmaster.c"
    "${VENDOR_JPEG_DIR}/jcomapi.c"
    "${VENDOR_JPEG_DIR}/jcparam.c"
    "${VENDOR_JPEG_DIR}/jcprepct.c"
    "${VENDOR_JPEG_DIR}/jcsample.c"
    "${VENDOR_JPEG_DIR}/jctrans.c"
    "${VENDOR_JPEG_DIR}/jdapimin.c"
    "${VENDOR_JPEG_DIR}/jdapistd.c"
    "${VENDOR_JPEG_DIR}/jdarith.c"
    "${VENDOR_JPEG_DIR}/jdatadst.c"
    "${VENDOR_JPEG_DIR}/jdatasrc.c"
    "${VENDOR_JPEG_DIR}/jdcoefct.c"
    "${VENDOR_JPEG_DIR}/jdcolor.c"
    "${VENDOR_JPEG_DIR}/jddctmgr.c"
    "${VENDOR_JPEG_DIR}/jdhuff.c"
    "${VENDOR_JPEG_DIR}/jdinput.c"
    "${VENDOR_JPEG_DIR}/jdmainct.c"
    "${VENDOR_JPEG_DIR}/jdmarker.c"
    "${VENDOR_JPEG_DIR}/jdmaster.c"
    "${VENDOR_JPEG_DIR}/jdmerge.c"
    "${VENDOR_JPEG_DIR}/jdpostct.c"
    "${VENDOR_JPEG_DIR}/jdsample.c"
    "${VENDOR_JPEG_DIR}/jdtrans.c"
    "${VENDOR_JPEG_DIR}/jerror.c"
    "${VENDOR_JPEG_DIR}/jfdctflt.c"
    "${VENDOR_JPEG_DIR}/jfdctfst.c"
    "${VENDOR_JPEG_DIR}/jfdctint.c"
    "${VENDOR_JPEG_DIR}/jidctflt.c"
    "${VENDOR_JPEG_DIR}/jidctfst.c"
    "${VENDOR_JPEG_DIR}/jidctint.c"
    "${VENDOR_JPEG_DIR}/jmemmgr.c"
    "${VENDOR_JPEG_DIR}/jmemnobs.c"
    "${VENDOR_JPEG_DIR}/jquant1.c"
    "${VENDOR_JPEG_DIR}/jquant2.c"
    "${VENDOR_JPEG_DIR}/jutils.c"
    "${VENDOR_JPEG_DIR}/jconfig.h"
    "${VENDOR_JPEG_DIR}/jdct.h"
    "${VENDOR_JPEG_DIR}/jerror.h"
    "${VENDOR_JPEG_DIR}/jmemsys.h"
    "${VENDOR_JPEG_DIR}/jmorecfg.h"
    "${VENDOR_JPEG_DIR}/jpegint.h"
    "${VENDOR_JPEG_DIR}/jpeglib.h"
    "${VENDOR_JPEG_DIR}/jversion.h"
)

target_include_directories(vendor-jpeg PUBLIC "${VENDOR_JPEG_DIR}")
