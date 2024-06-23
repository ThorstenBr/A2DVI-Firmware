cmake_minimum_required(VERSION 3.18)

# firmware version
set(FW_VERSION "0.0a2")
message(STATUS "Building firmware version ${FW_VERSION}")

set(BINARY_NAME "A2DVI_v${FW_VERSION}")

if (FEATURE_DEBUG)
message(STATUS "Building DEBUG version")
add_compile_options(-DFEATURE_DEBUG_NO6502)
set(BINARY_NAME "${BINARY_NAME}_DEBUG")
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

if (NOT PICO_SDK_PATH)
   message(FATAL_ERROR "PICO_SDK_PATH is not set! Provide PICO_SDK_PATH as an environment variable.")
else()
    include(${PICO_SDK_PATH}/pico_sdk_init.cmake)
endif ()

enable_language(C CXX ASM)
project(A2DVI C CXX ASM)

# enable compiler warnings
add_compile_options(-Wall -Wno-unused-function)

add_compile_options(-DDVI_N_TMDS_BUFFERS=5)
add_compile_options(-DFUNCTION_VGA=1)
add_compile_options(-DFW_VERSION="${FW_VERSION}")

pico_sdk_init()

include(../libraries/libdvi/CMakeLists.txt)

add_executable(${BINARY_NAME}
	main.c
	applebus/abus.c
	applebus/abus_setup.c
	applebus/buffers.c
	applebus/businterface.c
	dvi/a2dvi.c
	dvi/render_text.c
	config/config.c
	debug/debug.c
)

target_compile_options(${BINARY_NAME} PRIVATE -Wall)

target_link_libraries(${BINARY_NAME}
	pico_stdlib
	pico_multicore
	pico_util
	libdvi
)

target_include_directories(${BINARY_NAME} PUBLIC lib/PicoDVI/software/include)
target_include_directories(${BINARY_NAME} PUBLIC assets)

pico_generate_pio_header(${BINARY_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/applebus/abus.pio)

# create map/bin/hex file etc.
pico_add_extra_outputs(${BINARY_NAME})
