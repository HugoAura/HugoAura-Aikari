#pragma once

#include <windows.h>

namespace AikariShared::Utils::Windows::RC
{
    template <typename ResourceType>
    struct LoadResourceRet
    {
        bool success;
        ResourceType* retPtr;
        std::string message;
        DWORD size;
    };

    template <typename ResourceType>
    LoadResourceRet<ResourceType> loadStringResource(
        HINSTANCE targetHIns, const int& resId
    )
    {
        LoadResourceRet<ResourceType> ret = {
            .success = false, .retPtr = nullptr, .message = "", .size = 0
        };

        HRSRC hRes =
            FindResourceW(targetHIns, MAKEINTRESOURCEW(resId), RT_RCDATA);
        if (hRes == NULL)
        {
            ret.message = "Failed to find res. ResId: " + std::to_string(resId);
            return ret;
        }

        HGLOBAL hResLoad = LoadResource(targetHIns, hRes);
        if (hResLoad == NULL)
        {
            ret.message = "Failed to load res. ResId: " + std::to_string(resId);
            return ret;
        }

        void* resPtr = LockResource(hResLoad);
        if (resPtr == NULL)
        {
            ret.message = "Failed to lock res. ResId: " + std::to_string(resId);
            return ret;
        }

        DWORD resSize = SizeofResource(targetHIns, hRes);

        ret.retPtr = static_cast<ResourceType*>(resPtr);
        ret.success = true;
        ret.message = "Success";
        ret.size = resSize;

        return ret;
    };
}  // namespace AikariShared::utils::windows::rc
