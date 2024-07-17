cmake_minimum_required(VERSION 3.18)

# firmware version
set(FW_VERSION "0.6")
message(STATUS "Building firmware version ${FW_VERSION}")

set(BINARY_NAME "A2DVI_v${FW_VERSION}")

if (FEATURE_TEST)
    message(STATUS "Building TEST version")
    add_compile_options(-DFEATURE_TEST)
    set(BINARY_NAME "${BINARY_NAME}_TEST")
    set(TEST_SOURCES "test/tests.c")
else()
    message(STATUS "Building Release version")
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(FAMILY rp2040)
set(PICO_PLATFORM rp2040)
set(BOARD pico_sdk)

set(PICO_STDIO_UART OFF)
set(PICO_STDIO_USB  OFF)

# Pull in SDK (must be before project)
include(cmake/pico_sdk_import.cmake)

enable_language(C CXX ASM)
project(A2DVI C CXX ASM)

# enable compiler warnings
add_compile_options(-Wall -Wno-unused-function)

add_compile_options(-DDVI_N_TMDS_BUFFERS=5)
add_compile_options(-DFW_VERSION="${FW_VERSION}")

if (1)
add_compile_options(-DAPPLE_MODEL_IIE=1)
set(FONTS fonts/iie_us_enhanced.c fonts/iie_us_unenhanced.c fonts/iie_us_reactive.c fonts/iie_uk_enhanced.c fonts/iie_fr_ca_enhanced.c fonts/iie_de_enhanced.c fonts/iie_spanish_enhanced.c fonts/iie_it_enhanced.c fonts/iie_hebrew_enhanced.c fonts/iie_se_fi_enhanced.c fonts/clone_pravetz_cyrillic.c
 fonts/iiplus_us.c fonts/iiplus_videx_lowercase1.c fonts/iiplus_videx_lowercase2.c fonts/iiplus_pigfont.c fonts/iiplus_jp_katakana.c)
endif()

pico_sdk_init()

include(../libraries/libdvi/CMakeLists.txt)

add_executable(${BINARY_NAME}
	main.c
	applebus/abus.c
	applebus/abus_setup.c
	applebus/buffers.c
	applebus/businterface.c

	dvi/a2dvi.c
	dvi/tmds.c

	render/render.c
	render/render_status.c
	render/render_text.c
	render/render_lores.c
	render/render_dgr.c
	render/render_hires.c
	render/render_dhgr.c

	config/config.c
	config/device_regs.c

	menu/menu.c

	debug/debug.c
	util/dmacopy.c

	fonts/textfont.c
	${FONTS}

	${TEST_SOURCES}
)

target_compile_options(${BINARY_NAME} PRIVATE -Wall)

target_link_libraries(${BINARY_NAME}
	pico_stdlib
	pico_multicore
	pico_util
	hardware_flash
	libdvi
)

target_include_directories(${BINARY_NAME} PUBLIC lib/PicoDVI/software/include)
target_include_directories(${BINARY_NAME} PUBLIC assets .)

pico_generate_pio_header(${BINARY_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/applebus/abus.pio)

pico_set_binary_type(${BINARY_NAME} copy_to_ram)

# create map/bin/hex file etc.
pico_add_extra_outputs(${BINARY_NAME})

pico_set_linker_script(${BINARY_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/copy_to_ram_custom.ld)
