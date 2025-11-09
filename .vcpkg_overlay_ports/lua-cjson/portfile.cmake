vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO mpx/lua-cjson
        REF 718f27293a981fb5e9e662e9aec0b7cf78317da6
        SHA512 32ad9b83f6700dcaf2d12612df5d0da74178fe82ac79e71584d8117613630d3296bb3ae19d8c7feeedbf87dda77cacc67651c9c697cff98ee3cb9f1a1f52098f
        HEAD_REF master
        PATCHES
        CMakeLists.patch
        cjsonConfig.patch
)

set(VCPKG_POLICY_EMPTY_INCLUDE_FOLDER enabled)
set(VCPKG_POLICY_DLLS_WITHOUT_LIBS enabled)

if (VCPKG_TARGET_IS_WINDOWS)
    set(VCPKG_C_FLAGS "${VCPKG_C_FLAGS} /Dstrncasecmp=_strnicmp")
    set(VCPKG_CXX_FLAGS "${VCPKG_CXX_FLAGS}")
endif ()

vcpkg_cmake_configure(
        SOURCE_PATH "${SOURCE_PATH}"
        # OPTIONS
        # -DBUILD_MODULE=ON
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(
    PACKAGE_NAME cjson
    CONFIG_PATH share/cjson
)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")

file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
