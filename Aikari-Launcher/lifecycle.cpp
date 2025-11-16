#include "./lifecycle.h"

#include <Aikari-Launcher-Private/types/global/lifecycleTypes.h>

#include "components/config.h"
#include "components/wsServer.h"
#include "infrastructure/fileSystem.h"
#include "infrastructure/registry.h"

namespace AikariTypes::Global::Lifecycle
{
    // --- Begin implementations of
    // AikariTypes::global::lifecycle::SharedInstances
    SharedInstances::SharedInstances() = default;

    SharedInstances SharedInstances::createDefault()
    {
        SharedInstances result;
        return result;
    };

    SharedInstances::~SharedInstances() = default;
    SharedInstances::SharedInstances(SharedInstances&&) noexcept = default;
    SharedInstances& SharedInstances::operator=(SharedInstances&&) noexcept =
        default;
    // --- End implementations of
    // AikariTypes::global::lifecycle::SharedInstances

}  // namespace AikariTypes::Global::Lifecycle
