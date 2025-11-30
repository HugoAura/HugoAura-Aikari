# Credit: @flutydeer
# From: https://github.com/stdware/vcpkg-overlay/blob/main/ports/qwindowkit/portfile.cmake

vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO stdware/qwindowkit
        REF 4f59fc98058df144e0ec7bea0cc6b017f1dcc25d
        SHA512 8d6cc960da7c26005f60bd1c45e94cc8fb5749f2e5ef2ce16779677a33978b24a7b1674fc69ff990c863d9a60f9457290dddb5fb2dc27384ee24bdacdd6d2ee5
)

vcpkg_check_features(
        OUT_FEATURE_OPTIONS FEATURE_OPTIONS
        FEATURES
        widgets QWINDOWKIT_BUILD_WIDGETS
        quick QWINDOWKIT_BUILD_QUICK
)

vcpkg_cmake_configure(
        SOURCE_PATH "${SOURCE_PATH}"
        OPTIONS
        -DQWINDOWKIT_BUILD_STATIC=OFF
        ${FEATURE_OPTIONS}
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME ${PORT} CONFIG_PATH lib/cmake/QWindowKit)
vcpkg_copy_pdbs()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}/" RENAME copyright)
