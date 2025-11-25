#include <Aikari-Shared/infrastructure/loggerMacro.h>
#include <Aikari-Shared/infrastructure/telemetry.h>
#include <Aikari-Shared/infrastructure/telemetryShortFn.h>
#include <Aikari-Shared/types/constants/utils.h>
#include <Aikari-Shared/utils/network.h>  // self
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

namespace AikariShared::Utils::Network
{
    namespace DNS
    {
        std::vector<std::string> getDNSARecordResult(
            const std::string& targetDomain
        )
        {
            static const std::string& TELEMETRY_MODULE_NAME =
                "Shared - Network - GetDNS[A]";
            static const std::string& TELEMETRY_ACTION_CATEGORY =
                "shared.utils.network.getDnsARecord";
            std::vector<std::string> result;

            cpr::SslOptions sslOpts = cpr::Ssl(cpr::ssl::TLSv1_2());
            sslOpts.verify_host = true;
            sslOpts.verify_peer = false;
            for (const std::string& host :
                 AikariShared::Constants::Utils::Network::DNS::dohQueryHosts)
            {
                cpr::Response response = cpr::Get(
                    cpr::Url{ host },
                    cpr::Parameters{ { "name", targetDomain },
                                     { "type", "A" } },
                    sslOpts,
                    cpr::Timeout{ 1500 }
                );

                if (response.status_code != 200)
                {
                    LOG_WARN(
                        "DoH source {} returned a non-200 code: {}",
                        host,
                        response.status_code
                    );
                    Telemetry::addBreadcrumb(
                        "default",
                        std::format(
                            "DoH query failed for {} | CPR response code: {} |"
                            " DoH host: {}",
                            targetDomain,
                            response.status_code,
                            host
                        ),
                        TELEMETRY_ACTION_CATEGORY,
                        "warning"
                    );
                    continue;
                }

                nlohmann::json parsedRep;
                try
                {
                    parsedRep = nlohmann::json::parse(response.text);
                }
                catch (const std::exception& err)
                {
                    LOG_WARN(
                        "Failed to parse DoH query result: {}", err.what()
                    );
                    Telemetry::addBreadcrumb(
                        "default",
                        std::format(
                            "Error parsing JSON rep for DoH query. Domain: {} |"
                            "Error message: {} | DoH "
                            "host: {}",
                            targetDomain,
                            err.what(),
                            host
                        ),
                        TELEMETRY_ACTION_CATEGORY,
                        "warning"
                    );
                    continue;
                }

                if (!parsedRep.contains("Answer"))
                {
                    LOG_WARN("Failed to resolve DoH query result: No answer");
                    Telemetry::addBreadcrumb(
                        "default",
                        std::format(
                            "DoH query no answer. Domain: {} |"
                            "DoH host: {}",
                            targetDomain,
                            host
                        ),
                        TELEMETRY_ACTION_CATEGORY,
                        "warning"
                    );
                    continue;
                }
                if (!parsedRep["Answer"].is_array())
                {
                    LOG_WARN(
                        "Failed to resolve DoH query result: Malformed response"
                    );
                    Telemetry::addBreadcrumb(
                        "default",
                        std::format(
                            "DoH query rep malformed. Domain: {} |"
                            "DoH host: {} | Data: {}",
                            targetDomain,
                            host,
                            response.text
                        ),
                        TELEMETRY_ACTION_CATEGORY,
                        "warning"
                    );
                    continue;
                }

                for (const auto& answer : parsedRep["Answer"])
                {
                    const int answerType = answer["type"].get<int>();
                    if (answerType == 1)
                    {
#ifdef _DEBUG
                        LOG_TRACE(
                            "Found new DNS A record: {}",
                            answer["data"].get<std::string>()
                        );
#endif
                        result.emplace_back(answer["data"].get<std::string>());
                    }
                }

                return result;
            }

            Telemetry::sendEventStr(
                SENTRY_LEVEL_ERROR,
                TELEMETRY_MODULE_NAME,
                std::format(
                    "Tried all DoH sources but failed | "
                    "Target domain: {}",
                    targetDomain
                )
            );
            return {};
        };
    }  // namespace DNS
}  // namespace AikariShared::Utils::Network
