project "cryptopp"
	language "C++"
	kind "StaticLib"
	targetname "cryptopp"
	optimize "Full"
	exceptionhandling "SEH"

	vpaths {
		["Headers/*"] = "8.4.0/**.h",
		["Sources"] = "8.4.0/**.cpp",
		["*"] = "premake5.lua"
	}

	files {
		"premake5.lua",
		"8.4.0/cryptlib.cpp",
		"8.4.0/cpu.cpp",
		"8.4.0/integer.cpp",
		"8.4.0/3way.cpp",
		"8.4.0/adler32.cpp",
		"8.4.0/algebra.cpp",
		"8.4.0/algparam.cpp",
		"8.4.0/allocate.cpp",
		"8.4.0/arc4.cpp",
		"8.4.0/aria.cpp",
		"8.4.0/aria_simd.cpp",
		"8.4.0/ariatab.cpp",
		"8.4.0/asn.cpp",
		"8.4.0/authenc.cpp",
		"8.4.0/base32.cpp",
		"8.4.0/base64.cpp",
		"8.4.0/basecode.cpp",
		"8.4.0/bfinit.cpp",
		"8.4.0/blake2.cpp",
		"8.4.0/blake2s_simd.cpp",
		"8.4.0/blake2b_simd.cpp",
		"8.4.0/blowfish.cpp",
		"8.4.0/blumshub.cpp",
		"8.4.0/camellia.cpp",
		"8.4.0/cast.cpp",
		"8.4.0/casts.cpp",
		"8.4.0/cbcmac.cpp",
		"8.4.0/ccm.cpp",
		"8.4.0/chacha.cpp",
		"8.4.0/chacha_simd.cpp",
		"8.4.0/chachapoly.cpp",
		"8.4.0/cham.cpp",
		"8.4.0/cham_simd.cpp",
		"8.4.0/channels.cpp",
		"8.4.0/cmac.cpp",
		"8.4.0/crc.cpp",
		"8.4.0/crc_simd.cpp",
		"8.4.0/darn.cpp",
		"8.4.0/default.cpp",
		"8.4.0/des.cpp",
		"8.4.0/dessp.cpp",
		"8.4.0/dh.cpp",
		"8.4.0/dh2.cpp",
		"8.4.0/dll.cpp",
		"8.4.0/donna_32.cpp",
		"8.4.0/donna_64.cpp",
		"8.4.0/donna_sse.cpp",
		"8.4.0/dsa.cpp",
		"8.4.0/eax.cpp",
		"8.4.0/ec2n.cpp",
		"8.4.0/ecp.cpp",
		"8.4.0/elgamal.cpp",
		"8.4.0/emsa2.cpp",
		"8.4.0/esign.cpp",
		"8.4.0/files.cpp",
		"8.4.0/filters.cpp",
		"8.4.0/fips140.cpp",
		"8.4.0/fipstest.cpp",
		"8.4.0/gcm.cpp",
		"8.4.0/gcm_simd.cpp",
		"8.4.0/gf256.cpp",
		"8.4.0/gf2_32.cpp",
		"8.4.0/gf2n.cpp",
		"8.4.0/gf2n_simd.cpp",
		"8.4.0/gfpcrypt.cpp",
		"8.4.0/gost.cpp",
		"8.4.0/gzip.cpp",
		"8.4.0/hc128.cpp",
		"8.4.0/hc256.cpp",
		"8.4.0/hex.cpp",
		"8.4.0/hight.cpp",
		"8.4.0/hmac.cpp",
		"8.4.0/hrtimer.cpp",
		"8.4.0/ida.cpp",
		"8.4.0/idea.cpp",
		"8.4.0/iterhash.cpp",
		"8.4.0/kalyna.cpp",
		"8.4.0/kalynatab.cpp",
		"8.4.0/keccak.cpp",
		"8.4.0/keccak_core.cpp",
		"8.4.0/keccak_simd.cpp",
		"8.4.0/lea.cpp",
		"8.4.0/lea_simd.cpp",
		"8.4.0/luc.cpp",
		"8.4.0/mars.cpp",
		"8.4.0/marss.cpp",
		"8.4.0/md2.cpp",
		"8.4.0/md4.cpp",
		"8.4.0/md5.cpp",
		"8.4.0/misc.cpp",
		"8.4.0/modes.cpp",
		"8.4.0/mqueue.cpp",
		"8.4.0/mqv.cpp",
		"8.4.0/nbtheory.cpp",
		"8.4.0/oaep.cpp",
		"8.4.0/osrng.cpp",
		"8.4.0/padlkrng.cpp",
		"8.4.0/panama.cpp",
		"8.4.0/pch.cpp",
		"8.4.0/pkcspad.cpp",
		"8.4.0/poly1305.cpp",
		"8.4.0/polynomi.cpp",
		"8.4.0/pssr.cpp",
		"8.4.0/pubkey.cpp",
		"8.4.0/queue.cpp",
		"8.4.0/rabin.cpp",
		"8.4.0/randpool.cpp",
		"8.4.0/rabbit.cpp",
		"8.4.0/rc2.cpp",
		"8.4.0/rc5.cpp",
		"8.4.0/rc6.cpp",
		"8.4.0/rdrand.cpp",
		"8.4.0/rdtables.cpp",
		"8.4.0/rijndael.cpp",
		"8.4.0/rijndael_simd.cpp",
		"8.4.0/ripemd.cpp",
		"8.4.0/rng.cpp",
		"8.4.0/rsa.cpp",
		"8.4.0/rw.cpp",
		"8.4.0/safer.cpp",
		"8.4.0/salsa.cpp",
		"8.4.0/scrypt.cpp",
		"8.4.0/seal.cpp",
		"8.4.0/seed.cpp",
		"8.4.0/serpent.cpp",
		"8.4.0/sha.cpp",
		"8.4.0/sha_simd.cpp",
		"8.4.0/sha3.cpp",
		"8.4.0/shacal2.cpp",
		"8.4.0/shacal2_simd.cpp",
		"8.4.0/shake.cpp",
		"8.4.0/shark.cpp",
		"8.4.0/sharkbox.cpp",
		"8.4.0/simeck.cpp",
		"8.4.0/simon.cpp",
		"8.4.0/simon128_simd.cpp",
		"8.4.0/simple.cpp",
		"8.4.0/skipjack.cpp",
		"8.4.0/sm3.cpp",
		"8.4.0/sm4.cpp",
		"8.4.0/sm4_simd.cpp",
		"8.4.0/sosemanuk.cpp",
		"8.4.0/speck.cpp",
		"8.4.0/speck128_simd.cpp",
		"8.4.0/square.cpp",
		"8.4.0/squaretb.cpp",
		"8.4.0/sse_simd.cpp",
		"8.4.0/strciphr.cpp",
		"8.4.0/tea.cpp",
		"8.4.0/tftables.cpp",
		"8.4.0/threefish.cpp",
		"8.4.0/tiger.cpp",
		"8.4.0/tigertab.cpp",
		"8.4.0/ttmac.cpp",
		"8.4.0/tweetnacl.cpp",
		"8.4.0/twofish.cpp",
		"8.4.0/vmac.cpp",
		"8.4.0/wake.cpp",
		"8.4.0/whrlpool.cpp",
		"8.4.0/xed25519.cpp",
		"8.4.0/xtr.cpp",
		"8.4.0/xtrcrypt.cpp",
		"8.4.0/xts.cpp",
		"8.4.0/zdeflate.cpp",
		"8.4.0/zinflate.cpp",
		"8.4.0/zlib.cpp",
		"8.4.0/3way.h",
		"8.4.0/adler32.h",
		"8.4.0/adv_simd.h",
		"8.4.0/aes.h",
		"8.4.0/algebra.h",
		"8.4.0/algparam.h",
		"8.4.0/allocate.h",
		"8.4.0/arc4.h",
		"8.4.0/aria.h",
		"8.4.0/argnames.h",
		"8.4.0/asn.h",
		"8.4.0/authenc.h",
		"8.4.0/base32.h",
		"8.4.0/base64.h",
		"8.4.0/basecode.h",
		"8.4.0/blake2.h",
		"8.4.0/blowfish.h",
		"8.4.0/blumshub.h",
		"8.4.0/camellia.h",
		"8.4.0/cast.h",
		"8.4.0/cbcmac.h",
		"8.4.0/ccm.h",
		"8.4.0/chacha.h",
		"8.4.0/chachapoly.h",
		"8.4.0/cham.h",
		"8.4.0/channels.h",
		"8.4.0/cmac.h",
		"8.4.0/config.h",
		"8.4.0/config_align.h",
		"8.4.0/config_asm.h",
		"8.4.0/config_cpu.h",
		"8.4.0/config_cxx.h",
		"8.4.0/config_dll.h",
		"8.4.0/config_int.h",
		"8.4.0/config_misc.h",
		"8.4.0/config_ns.h",
		"8.4.0/config_os.h",
		"8.4.0/config_ver.h",
		"8.4.0/cpu.h",
		"8.4.0/crc.h",
		"8.4.0/cryptlib.h",
		"8.4.0/darn.h",
		"8.4.0/default.h",
		"8.4.0/des.h",
		"8.4.0/dh.h",
		"8.4.0/dh2.h",
		"8.4.0/dll.h",
		"8.4.0/dmac.h",
		"8.4.0/drbg.h",
		"8.4.0/donna.h",
		"8.4.0/donna_32.h",
		"8.4.0/donna_64.h",
		"8.4.0/donna_sse.h",
		"8.4.0/dsa.h",
		"8.4.0/eax.h",
		"8.4.0/ec2n.h",
		"8.4.0/eccrypto.h",
		"8.4.0/ecp.h",
		"8.4.0/ecpoint.h",
		"8.4.0/elgamal.h",
		"8.4.0/emsa2.h",
		"8.4.0/eprecomp.h",
		"8.4.0/esign.h",
		"8.4.0/files.h",
		"8.4.0/filters.h",
		"8.4.0/fips140.h",
		"8.4.0/fhmqv.h",
		"8.4.0/fltrimpl.h",
		"8.4.0/gcm.h",
		"8.4.0/gf256.h",
		"8.4.0/gf2_32.h",
		"8.4.0/gf2n.h",
		"8.4.0/gfpcrypt.h",
		"8.4.0/gost.h",
		"8.4.0/gzip.h",
		"8.4.0/hc128.h",
		"8.4.0/hc256.h",
		"8.4.0/hex.h",
		"8.4.0/hight.h",
		"8.4.0/hkdf.h",
		"8.4.0/hmac.h",
		"8.4.0/hmqv.h",
		"8.4.0/hrtimer.h",
		"8.4.0/ida.h",
		"8.4.0/idea.h",
		"8.4.0/integer.h",
		"8.4.0/iterhash.h",
		"8.4.0/kalyna.h",
		"8.4.0/keccak.h",
		"8.4.0/lubyrack.h",
		"8.4.0/lea.h",
		"8.4.0/luc.h",
		"8.4.0/mars.h",
		"8.4.0/md2.h",
		"8.4.0/md4.h",
		"8.4.0/md5.h",
		"8.4.0/mdc.h",
		"8.4.0/misc.h",
		"8.4.0/modarith.h",
		"8.4.0/modes.h",
		"8.4.0/modexppc.h",
		"8.4.0/mqueue.h",
		"8.4.0/mqv.h",
		"8.4.0/naclite.h",
		"8.4.0/nbtheory.h",
		"8.4.0/nr.h",
		"8.4.0/oaep.h",
		"8.4.0/oids.h",
		"8.4.0/osrng.h",
		"8.4.0/padlkrng.h",
		"8.4.0/panama.h",
		"8.4.0/pch.h",
		"8.4.0/pkcspad.h",
		"8.4.0/poly1305.h",
		"8.4.0/polynomi.h",
		"8.4.0/pssr.h",
		"8.4.0/pubkey.h",
		"8.4.0/pwdbased.h",
		"8.4.0/queue.h",
		"8.4.0/rabin.h",
		"8.4.0/randpool.h",
		"8.4.0/rabbit.h",
		"8.4.0/rc2.h",
		"8.4.0/rc5.h",
		"8.4.0/rc6.h",
		"8.4.0/rdrand.h",
		"8.4.0/rijndael.h",
		"8.4.0/ripemd.h",
		"8.4.0/rng.h",
		"8.4.0/rsa.h",
		"8.4.0/rw.h",
		"8.4.0/safer.h",
		"8.4.0/salsa.h",
		"8.4.0/scrypt.h",
		"8.4.0/seal.h",
		"8.4.0/secblock.h",
		"8.4.0/secblockfwd.h",
		"8.4.0/seckey.h",
		"8.4.0/seed.h",
		"8.4.0/serpent.h",
		"8.4.0/sha.h",
		"8.4.0/sha3.h",
		"8.4.0/shacal2.h",
		"8.4.0/shake.h",
		"8.4.0/shark.h",
		"8.4.0/simple.h",
		"8.4.0/simeck.h",
		"8.4.0/simon.h",
		"8.4.0/siphash.h",
		"8.4.0/skipjack.h",
		"8.4.0/sm3.h",
		"8.4.0/sm4.h",
		"8.4.0/smartptr.h",
		"8.4.0/sosemanuk.h",
		"8.4.0/speck.h",
		"8.4.0/square.h",
		"8.4.0/stdcpp.h",
		"8.4.0/strciphr.h",
		"8.4.0/tea.h",
		"8.4.0/threefish.h",
		"8.4.0/tiger.h",
		"8.4.0/trap.h",
		"8.4.0/trunhash.h",
		"8.4.0/ttmac.h",
		"8.4.0/tweetnacl.h",
		"8.4.0/twofish.h",
		"8.4.0/vmac.h",
		"8.4.0/wake.h",
		"8.4.0/whrlpool.h",
		"8.4.0/words.h",
		"8.4.0/xed25519.h",
		"8.4.0/xtr.h",
		"8.4.0/xtrcrypt.h",
		"8.4.0/xts.h",
		"8.4.0/zdeflate.h",
		"8.4.0/zinflate.h",
		"8.4.0/zlib.h",
	}

	pchheader "pch.h"
	pchsource "8.4.0/pch.cpp"

	defines {
		"USE_PRECOMPILED_HEADERS",
		"CRYPTOPP_DISABLE_ASM",
	}

	filter "files:8.4.0/iterhash.cpp or files:8.4.0/dll.cpp"
    	flags { "NoPCH" }

	filter "configurations:Release"
		defines { "NDEBUG" }

	filter "system:windows"
		disablewarnings {
			"4231",
			"4251",
			"4275",
			"4355",
			"4505",
		}
