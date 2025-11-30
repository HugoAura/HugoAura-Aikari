# Dep for qwindowkit
# Credit: @flutydeer
# From: https://github.com/stdware/vcpkg-overlay/blob/main/ports/qmsetup/portfile.cmake

vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO stdware/qmsetup
        REF 2f10ebc3723a9b03edf309611483ee643f10add7
        SHA512 a1a6e4d60f68ce910a4f897beca870109ff5fead250e3e194051e1760ab62c1a1ad3ef6b58d20f5cb08da8c50121c3d55da3f2b8600c99d3b96289613fb42214
)

vcpkg_cmake_configure(
        SOURCE_PATH "${SOURCE_PATH}"
        OPTIONS
        -DQMSETUP_VCPKG_TOOLS_HINT=ON
        -DQMSETUP_STATIC_RUNTIME=OFF
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(PACKAGE_NAME ${PORT}
        CONFIG_PATH lib/cmake/${PORT}
)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/lib")
file(MAKE_DIRECTORY "${CURRENT_PACKAGES_DIR}/debug")
file(COPY "${CURRENT_PACKAGES_DIR}/tools" DESTINATION "${CURRENT_PACKAGES_DIR}/debug")

vcpkg_copy_pdbs()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}/" RENAME copyright)
