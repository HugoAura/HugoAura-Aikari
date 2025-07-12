#include <Aikari-Shared/infrastructure/loggerMacro.h>
#include <Aikari-Shared/types/constants/utils.h>
#include <Aikari-Shared/utils/network.h>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

namespace AikariShared::utils::network
{
    namespace DNS
    {
        std::vector<std::string> getDNSARecordResult(
            const std::string& targetDomain
        )
        {
            std::vector<std::string> result;

            cpr::SslOptions sslOpts = cpr::Ssl(cpr::ssl::TLSv1_3());
            sslOpts.verify_host = true;
            sslOpts.verify_peer = false;
            cpr::Response response = cpr::Get(
                cpr::Url{ AikariShared::constants::utils::network::DNS::
                              defaultDoHQueryHost },
                cpr::Parameters{ { "name", targetDomain }, { "type", "A" } },
                sslOpts
            );

            if (response.status_code != 200)
            {
                LOG_WARN(
                    "DoH query returned a non-200 code: {}",
                    response.status_code
                );
                return result;
            }

            nlohmann::json parsedRep;
            try
            {
                parsedRep = nlohmann::json::parse(response.text);
            }
            catch (const std::exception& err)
            {
                LOG_WARN("Failed to parse DoH query result: {}", err.what());
                return result;
            }

            if (!parsedRep.contains("Answer"))
            {
                LOG_WARN("Failed to resolve DoH query result: No answer");
                return result;
            }
            if (!parsedRep["Answer"].is_array())
            {
                LOG_WARN(
                    "Failed to resolve DoH query result: Malformed response"
                );
                return result;
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
        };
    }  // namespace DNS
}  // namespace AikariShared::utils::network
