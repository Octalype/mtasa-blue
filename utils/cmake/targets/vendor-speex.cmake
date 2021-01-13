#
# speex library target
#
if (NOT (MTASA_OS_WINDOWS AND MTASA_X86))
    return()
endif()

set(VENDOR_SPEEX_DIR "${MTASA_VENDOR_DIR}/libspeex")

add_library(vendor-speex STATIC
    "${VENDOR_SPEEX_DIR}/libspeex/arch.h"
    "${VENDOR_SPEEX_DIR}/libspeex/bfin.h"
    "${VENDOR_SPEEX_DIR}/libspeex/bits.c"
    "${VENDOR_SPEEX_DIR}/libspeex/cb_search.c"
    "${VENDOR_SPEEX_DIR}/libspeex/cb_search.h"
    "${VENDOR_SPEEX_DIR}/libspeex/cb_search_arm4.h"
    "${VENDOR_SPEEX_DIR}/libspeex/cb_search_bfin.h"
    "${VENDOR_SPEEX_DIR}/libspeex/cb_search_sse.h"
    "${VENDOR_SPEEX_DIR}/libspeex/config.h"
    "${VENDOR_SPEEX_DIR}/libspeex/exc_10_16_table.c"
    "${VENDOR_SPEEX_DIR}/libspeex/exc_10_32_table.c"
    "${VENDOR_SPEEX_DIR}/libspeex/exc_20_32_table.c"
    "${VENDOR_SPEEX_DIR}/libspeex/exc_5_256_table.c"
    "${VENDOR_SPEEX_DIR}/libspeex/exc_5_64_table.c"
    "${VENDOR_SPEEX_DIR}/libspeex/exc_8_128_table.c"
    "${VENDOR_SPEEX_DIR}/libspeex/fftwrap.h"
    "${VENDOR_SPEEX_DIR}/libspeex/filters.c"
    "${VENDOR_SPEEX_DIR}/libspeex/filters.h"
    "${VENDOR_SPEEX_DIR}/libspeex/filters_arm4.h"
    "${VENDOR_SPEEX_DIR}/libspeex/filters_bfin.h"
    "${VENDOR_SPEEX_DIR}/libspeex/filters_sse.h"
    "${VENDOR_SPEEX_DIR}/libspeex/fixed_arm4.h"
    "${VENDOR_SPEEX_DIR}/libspeex/fixed_arm5e.h"
    "${VENDOR_SPEEX_DIR}/libspeex/fixed_bfin.h"
    "${VENDOR_SPEEX_DIR}/libspeex/fixed_debug.h"
    "${VENDOR_SPEEX_DIR}/libspeex/fixed_generic.h"
    "${VENDOR_SPEEX_DIR}/libspeex/gain_table.c"
    "${VENDOR_SPEEX_DIR}/libspeex/gain_table_lbr.c"
    "${VENDOR_SPEEX_DIR}/libspeex/hexc_10_32_table.c"
    "${VENDOR_SPEEX_DIR}/libspeex/hexc_table.c"
    "${VENDOR_SPEEX_DIR}/libspeex/high_lsp_tables.c"
    "${VENDOR_SPEEX_DIR}/libspeex/kiss_fft.c"
    "${VENDOR_SPEEX_DIR}/libspeex/kiss_fft.h"
    "${VENDOR_SPEEX_DIR}/libspeex/kiss_fftr.c"
    "${VENDOR_SPEEX_DIR}/libspeex/kiss_fftr.h"
    "${VENDOR_SPEEX_DIR}/libspeex/lpc.c"
    "${VENDOR_SPEEX_DIR}/libspeex/lpc.h"
    "${VENDOR_SPEEX_DIR}/libspeex/lpc_bfin.h"
    "${VENDOR_SPEEX_DIR}/libspeex/lsp.c"
    "${VENDOR_SPEEX_DIR}/libspeex/lsp.h"
    "${VENDOR_SPEEX_DIR}/libspeex/lsp_bfin.h"
    "${VENDOR_SPEEX_DIR}/libspeex/lsp_tables_nb.c"
    "${VENDOR_SPEEX_DIR}/libspeex/ltp.c"
    "${VENDOR_SPEEX_DIR}/libspeex/ltp.h"
    "${VENDOR_SPEEX_DIR}/libspeex/ltp_arm4.h"
    "${VENDOR_SPEEX_DIR}/libspeex/ltp_bfin.h"
    "${VENDOR_SPEEX_DIR}/libspeex/ltp_sse.h"
    "${VENDOR_SPEEX_DIR}/libspeex/math_approx.h"
    "${VENDOR_SPEEX_DIR}/libspeex/misc_bfin.h"
    "${VENDOR_SPEEX_DIR}/libspeex/modes.c"
    "${VENDOR_SPEEX_DIR}/libspeex/modes.h"
    "${VENDOR_SPEEX_DIR}/libspeex/modes_wb.c"
    "${VENDOR_SPEEX_DIR}/libspeex/nb_celp.c"
    "${VENDOR_SPEEX_DIR}/libspeex/nb_celp.h"
    "${VENDOR_SPEEX_DIR}/libspeex/os_support.h"
    "${VENDOR_SPEEX_DIR}/libspeex/quant_lsp.c"
    "${VENDOR_SPEEX_DIR}/libspeex/quant_lsp.h"
    "${VENDOR_SPEEX_DIR}/libspeex/quant_lsp_bfin.h"
    "${VENDOR_SPEEX_DIR}/libspeex/sb_celp.c"
    "${VENDOR_SPEEX_DIR}/libspeex/sb_celp.h"
    "${VENDOR_SPEEX_DIR}/libspeex/smallft.c"
    "${VENDOR_SPEEX_DIR}/libspeex/smallft.h"
    "${VENDOR_SPEEX_DIR}/libspeex/speex.c"
    "${VENDOR_SPEEX_DIR}/libspeex/speex_callbacks.c"
    "${VENDOR_SPEEX_DIR}/libspeex/speex_header.c"
    "${VENDOR_SPEEX_DIR}/libspeex/stack_alloc.h"
    "${VENDOR_SPEEX_DIR}/libspeex/stereo.c"
    "${VENDOR_SPEEX_DIR}/libspeex/vbr.c"
    "${VENDOR_SPEEX_DIR}/libspeex/vbr.h"
    "${VENDOR_SPEEX_DIR}/libspeex/vorbis_psy.c"
    "${VENDOR_SPEEX_DIR}/libspeex/vorbis_psy.h"
    "${VENDOR_SPEEX_DIR}/libspeex/vq.c"
    "${VENDOR_SPEEX_DIR}/libspeex/vq.h"
    "${VENDOR_SPEEX_DIR}/libspeex/vq_arm4.h"
    "${VENDOR_SPEEX_DIR}/libspeex/vq_bfin.h"
    "${VENDOR_SPEEX_DIR}/libspeex/vq_sse.h"
    "${VENDOR_SPEEX_DIR}/libspeex/window.c"
    "${VENDOR_SPEEX_DIR}/libspeex/_kiss_fft_guts.h"
)

target_include_directories(vendor-speex PUBLIC "${VENDOR_SPEEX_DIR}")

target_compile_definitions(vendor-speex PUBLIC HAVE_CONFIG_H)

add_library(vendor-speex-dsp STATIC
    "${VENDOR_SPEEX_DIR}/libspeexdsp/arch.h"
    "${VENDOR_SPEEX_DIR}/libspeexdsp/bfin.h"
    "${VENDOR_SPEEX_DIR}/libspeexdsp/buffer.c"
    "${VENDOR_SPEEX_DIR}/libspeexdsp/config.h"
    "${VENDOR_SPEEX_DIR}/libspeexdsp/fftwrap.c"
    "${VENDOR_SPEEX_DIR}/libspeexdsp/fftwrap.h"
    "${VENDOR_SPEEX_DIR}/libspeexdsp/filterbank.c"
    "${VENDOR_SPEEX_DIR}/libspeexdsp/filterbank.h"
    "${VENDOR_SPEEX_DIR}/libspeexdsp/fixed_arm4.h"
    "${VENDOR_SPEEX_DIR}/libspeexdsp/fixed_arm5e.h"
    "${VENDOR_SPEEX_DIR}/libspeexdsp/fixed_bfin.h"
    "${VENDOR_SPEEX_DIR}/libspeexdsp/fixed_debug.h"
    "${VENDOR_SPEEX_DIR}/libspeexdsp/fixed_generic.h"
    "${VENDOR_SPEEX_DIR}/libspeexdsp/jitter.c"
    "${VENDOR_SPEEX_DIR}/libspeexdsp/kiss_fft.c"
    "${VENDOR_SPEEX_DIR}/libspeexdsp/kiss_fft.h"
    "${VENDOR_SPEEX_DIR}/libspeexdsp/kiss_fftr.c"
    "${VENDOR_SPEEX_DIR}/libspeexdsp/kiss_fftr.h"
    "${VENDOR_SPEEX_DIR}/libspeexdsp/math_approx.h"
    "${VENDOR_SPEEX_DIR}/libspeexdsp/mdf.c"
    "${VENDOR_SPEEX_DIR}/libspeexdsp/misc_bfin.h"
    "${VENDOR_SPEEX_DIR}/libspeexdsp/os_support.h"
    "${VENDOR_SPEEX_DIR}/libspeexdsp/preprocess.c"
    "${VENDOR_SPEEX_DIR}/libspeexdsp/pseudofloat.h"
    "${VENDOR_SPEEX_DIR}/libspeexdsp/resample.c"
    "${VENDOR_SPEEX_DIR}/libspeexdsp/resample_neon.h"
    "${VENDOR_SPEEX_DIR}/libspeexdsp/resample_sse.h"
    "${VENDOR_SPEEX_DIR}/libspeexdsp/scal.c"
    "${VENDOR_SPEEX_DIR}/libspeexdsp/smallft.c"
    "${VENDOR_SPEEX_DIR}/libspeexdsp/smallft.h"
    "${VENDOR_SPEEX_DIR}/libspeexdsp/stack_alloc.h"
    "${VENDOR_SPEEX_DIR}/libspeexdsp/vorbis_psy.h"
    "${VENDOR_SPEEX_DIR}/libspeexdsp/_kiss_fft_guts.h"
)

target_compile_definitions(vendor-speex-dsp PUBLIC HAVE_CONFIG_H)

target_include_directories(vendor-speex-dsp PUBLIC "${VENDOR_SPEEX_DIR}")
