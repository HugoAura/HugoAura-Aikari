#pragma once

namespace AikariLauncherPublic::Constants::InterThread::FileSystem
{
    constexpr const char* _PREFIX = "fs";
    namespace Base
    {
        constexpr const char* _PREFIX = "base";

        constexpr const char* GET_DIR = "fs.base.getDir";
        /*
        GetDir -> 获取目录信息
        Data IN ←
        {
            "dirType": "auraRoot" | "aikariRoot" | "aikariConf" | "aikariLog"
        }
        Data REP →
        {
            "success": bool,
            "path": std::string <ONLY IF SUCCESS>,
            "diagnoseCode": <ONLY IF !SUCCESS>
        }
        */
    }  // namespace Base
}  // namespace AikariLauncherPublic::Constants::InterThread::FileSystem
