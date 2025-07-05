include(vcpkg_common_functions)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO Minoricew/paho.mqtt.embedded-c
    REF "ab1ba9a7ab7c3420f4c45ff002fc1796333ded10"
    SHA512 104348317cc911b1be0b4af005dffbc643a11a1b7d7b9954a62a2cdfc19cf7a3800d3f98622eb589be496442fd8af573403fdeb1456ec9c67556549603a99a86
    HEAD_REF mqttv5
)

vcpkg_cmake_configure(
    SOURCE_PATH ${SOURCE_PATH}
)

message(STATUS "Building ${PORT}...")
vcpkg_build_cmake()
message(STATUS "Building ${PORT}... done")

message(STATUS "Installing header files for ${PORT}...")
file(INSTALL "${SOURCE_PATH}/include/"
     DESTINATION "${CURRENT_PACKAGES_DIR}/include"
)

file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
