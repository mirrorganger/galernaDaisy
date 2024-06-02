cmake_minimum_required(VERSION 3.0.0)
project(GalernaDaisyTest VERSION 0.1.0 LANGUAGES C CXX ASM)

include(cmake/daisy-setup.cmake)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(${LIBDAISY_DIR} libdaisy)
add_subdirectory(${DAISYSP_DIR} DaisySP)
add_subdirectory(GalernaDaisy)
add_subdirectory(Utilities)
add_subdirectory(Dsp)
add_subdirectory(BasicTestApp)
add_subdirectory(ThxSeedApp)
add_subdirectory(SchroederReverbApp)




