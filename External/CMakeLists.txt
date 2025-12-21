include(ExternalProject)

# >>> Gamma Ray >>> #
if (EXTERNAL_BUILD_GAMMARAY)
    message(STATUS "GammaRay will be built")
    set(GAMMARAY_OUTPUT_DIR "${CMAKE_BINARY_DIR}/artifacts/tools/gammaray")

    ExternalProject_Add(
            External_GammaRay
            PREFIX "${CMAKE_BINARY_DIR}/_deps/gammaray"
            INSTALL_DIR "${GAMMARAY_OUTPUT_DIR}"
            GIT_REPOSITORY https://github.com/KDAB/GammaRay.git
            GIT_TAG 70b995b31b4702ab79733ead02ccd7f25daef8f2
            GIT_SHALLOW TRUE

            CMAKE_ARGS
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}

            -DGAMMARAY_ENABLE_SQUISH_SUPPORT=OFF
            -DGAMMARAY_BUILD_UI=ON
            -DGAMMARAY_WITH_KDSME=OFF
            -DGAMMARAY_USE_PCH=ON
            -DGAMMARAY_DISABLE_FEEDBACK=ON
            -DGAMMARAY_BUILD_DOCS=OFF
            -DGAMMARAY_MULTI_BUILD=OFF
            -DGAMMARAY_INSTALL_QT_LAYOUT=OFF
            -DGAMMARAY_CLIENT_ONLY_BUILD=OFF

            -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
            -DVCPKG_TARGET_TRIPLET=${VCPKG_TARGET_TRIPLET}

            -GNinja
    )
endif ()
# <<< Gamma Ray <<< #
