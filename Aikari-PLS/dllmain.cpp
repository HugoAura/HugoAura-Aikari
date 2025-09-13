#include <windows.h>

#include "./lifecycle.h"

BOOL APIENTRY
DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        {
            auto& sharedInsMgr =
                AikariPLS::Lifecycle::PLSSharedInsManager::getInstance();
            sharedInsMgr.setVal(
                &AikariPLS::Types::lifecycle::PLSSharedIns::hModuleIns, hModule
            );
            break;
        }
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
        default:
            break;
    }
    return TRUE;
}
