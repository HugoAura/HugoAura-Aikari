#pragma once

#include <Aikari-Launcher-Private/common.h>
#include <filesystem>
#include <string>

namespace AikariUtils::SSLUtils
{
    bool checkCertExists(
        std::filesystem::path &certPath, std::filesystem::path &keyPath
    );
    int genEC256TlsCert(
        std::filesystem::path &baseDir,
        std::string &certHost,
        std::string &certIdentifier
    );
    bool initWsCert(std::filesystem::path &baseDir, bool force);
}  // namespace AikariUtils::SSLUtils
