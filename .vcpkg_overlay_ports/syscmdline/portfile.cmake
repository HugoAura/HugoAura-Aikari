# Dep for qmsetup
# Credit: @SineStriker
# From: https://github.com/stdware/vcpkg-overlay/blob/main/ports/syscmdline/portfile.cmake

vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO SineStriker/syscmdline
        REF 5a67673ff96acbfd894ea653fbaca872fded758a
        SHA512 186670c4e0d36760474c88f27f6e3bff88ff30557a0106cae64fca8c8132354c3534b716471bdd87fbd0453b7a82bbb6f7c53b5c9896b2b6be5dfa7fe8d02ae9
)

vcpkg_cmake_configure(
        SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME ${PORT} CONFIG_PATH lib/cmake/${PORT})
vcpkg_copy_pdbs()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
