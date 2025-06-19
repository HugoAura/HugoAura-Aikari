#pragma once

#include <Aikari-Launcher-Private/common.h>

#include <filesystem>

namespace AikariUtils::sslUtils
{
bool checkCertExists(std::filesystem::path &certPath, std::filesystem::path &keyPath);
int genEC256TlsCert(std::filesystem::path &baseDir, std::string &certHost);
bool initWsCert(std::filesystem::path &baseDir);
}  // namespace AikariUtils::sslUtils
