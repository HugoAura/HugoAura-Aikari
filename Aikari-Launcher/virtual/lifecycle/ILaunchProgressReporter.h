#pragma once

namespace AikariLauncher::Virtual::Lifecycle
{
    typedef std::function<void(
        bool isFailed,
        bool isCompleted,
        bool isStarting,
        unsigned int waitForMs,
        unsigned int percent,
        unsigned int exitCode
    )>
        GenericReportFn;

    class IProgressReporter
    {
       public:
        virtual ~IProgressReporter() = default;
        virtual void reportLaunchProgress(
            bool isFailed,
            bool isCompleted,
            bool isStarting,
            unsigned int waitForMs,
            unsigned int percent,
            unsigned int exitCode
        ) = 0;
    };
}  // namespace AikariLauncher::Virtual::Lifecycle
