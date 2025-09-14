#include "sslUtils.h"

#include <chrono>
#include <fstream>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/ecdh.h>
#include <mbedtls/entropy.h>
#include <mbedtls/error.h>
#include <mbedtls/oid.h>
#include <mbedtls/pem.h>
#include <mbedtls/pk.h>
#include <mbedtls/x509_crt.h>

#include "../components/config.h"
#include "lifecycleUtils.h"

namespace AikariUtils::SSLUtils
{
    bool checkCertExists(
        std::filesystem::path &certPath, std::filesystem::path &keyPath
    )
    {
        auto isCertExists = std::filesystem::exists(certPath);
        auto isKeyExists = std::filesystem::exists(keyPath);
        if (!isCertExists || !isKeyExists)
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    int genEC256TlsCert(
        std::filesystem::path &baseDir,
        std::string &certHost,
        std::string &certIdentifier
    )
    {
        mbedtls_pk_context keyContainer;
        mbedtls_entropy_context entropy;
        mbedtls_ctr_drbg_context ctrDrbg;
        mbedtls_x509write_cert crtCtx;
        mbedtls_mpi serial;

        mbedtls_pk_init(&keyContainer);
        mbedtls_entropy_init(&entropy);
        mbedtls_ctr_drbg_init(&ctrDrbg);
        mbedtls_x509write_crt_init(&crtCtx);
        mbedtls_mpi_init(&serial);

        std::filesystem::create_directories(baseDir);

        auto cleanUp = [&keyContainer, &entropy, &ctrDrbg, &crtCtx, &serial]()
        {
            mbedtls_x509write_crt_free(&crtCtx);
            mbedtls_pk_free(&keyContainer);
            mbedtls_ctr_drbg_free(&ctrDrbg);
            mbedtls_entropy_free(&entropy);
            mbedtls_mpi_free(&serial);
        };

        int tmpRetVar = 0;
        try
        {
            const char *pers = "AikariX";
            tmpRetVar = mbedtls_ctr_drbg_seed(
                &ctrDrbg,
                mbedtls_entropy_func,
                &entropy,
                (const unsigned char *)pers,
                strlen(pers)
            );
            if (tmpRetVar != 0)
            {
                throw std::runtime_error(
                    "Error initializing ctrDrbg with seed."
                );
            }

            tmpRetVar = mbedtls_pk_setup(
                &keyContainer, mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY)
            );
            if (tmpRetVar != 0)
            {
                throw std::runtime_error("Error setting up keyContainer.");
            }

            tmpRetVar = mbedtls_ecp_gen_key(
                MBEDTLS_ECP_DP_SECP256R1,
                mbedtls_pk_ec(keyContainer),
                mbedtls_ctr_drbg_random,
                &ctrDrbg
            );
            if (tmpRetVar != 0)
            {
                throw std::runtime_error("Error generating EC Key.");
            }

            std::string issuerName = "CN=Project HugoAura,O=HugoAura,C=HA";
            std::string subjectName =
                "CN=HugoAura Aikari,O=HugoAura Aikari,C=HA";

            tmpRetVar = mbedtls_x509write_crt_set_subject_name(
                &crtCtx, subjectName.c_str()
            );
            if (tmpRetVar != 0)
            {
                throw std::runtime_error(
                    "Failed to write subject name to crt context."
                );
            }

            mbedtls_x509write_crt_set_issuer_name(&crtCtx, issuerName.c_str());

            mbedtls_mpi_read_string(&serial, 10, "1");
            tmpRetVar = mbedtls_x509write_crt_set_serial(&crtCtx, &serial);
            if (tmpRetVar != 0)
            {
                throw std::runtime_error("Failed to write serial to crtCtx.");
            }

            auto curTime = std::chrono::floor<std::chrono::seconds>(
                std::chrono::system_clock::now()
            );
            auto crtExpireTime = std::chrono::utc_clock::from_sys(curTime) +
                                 std::chrono::years(15);

            std::string curTimeStr = std::format("{:%Y%m%d%H%M%S}", curTime);
            std::string crtExpireTimeStr =
                std::format("{:%Y%m%d%H%M%S}", crtExpireTime);

            tmpRetVar = mbedtls_x509write_crt_set_validity(
                &crtCtx, curTimeStr.c_str(), crtExpireTimeStr.c_str()
            );
            if (tmpRetVar != 0)
            {
                throw std::runtime_error("Failed to set cert validity.");
            }

            mbedtls_x509write_crt_set_issuer_key(&crtCtx, &keyContainer);
            mbedtls_x509write_crt_set_subject_key(&crtCtx, &keyContainer);

            mbedtls_x509write_crt_set_version(
                &crtCtx, MBEDTLS_X509_CRT_VERSION_3
            );
            mbedtls_x509write_crt_set_md_alg(&crtCtx, MBEDTLS_MD_SHA256);

            tmpRetVar =
                mbedtls_x509write_crt_set_basic_constraints(&crtCtx, 0, -1);
            if (tmpRetVar != 0)
            {
                throw std::runtime_error(
                    "Failed to set crt basic constraints."
                );
            }

            tmpRetVar = mbedtls_x509write_crt_set_key_usage(
                &crtCtx,
                MBEDTLS_X509_KU_DIGITAL_SIGNATURE |
                    MBEDTLS_X509_KU_KEY_AGREEMENT
            );
            if (tmpRetVar != 0)
            {
                throw std::runtime_error("Failed to set key usage.");
            }

            mbedtls_x509_san_list sanList;
            memset(&sanList, 0, sizeof(sanList));
            sanList.node.type = MBEDTLS_X509_SAN_DNS_NAME;
            sanList.node.san.unstructured_name.p =
                (unsigned char *)certHost.c_str();
            sanList.node.san.unstructured_name.len = strlen(certHost.c_str());

            tmpRetVar = mbedtls_x509write_crt_set_subject_alternative_name(
                &crtCtx, &sanList
            );
            if (tmpRetVar != 0)
            {
                throw std::runtime_error("Failed to set SAN.");
            }

            std::filesystem::path keyPath =
                baseDir / std::format("{}.key", certIdentifier);
            std::filesystem::path crtPath =
                baseDir / std::format("{}.crt", certIdentifier);

            {
                unsigned char keyBuffer[1000];
                tmpRetVar = mbedtls_pk_write_key_pem(
                    &keyContainer, keyBuffer, sizeof(keyBuffer)
                );
                if (tmpRetVar != 0)
                {
                    throw std::runtime_error(
                        "Failed to write key into buffer."
                    );
                }

                if (std::filesystem::exists(keyPath))
                {
                    std::filesystem::remove(keyPath);
                }

                std::ofstream keyFile(keyPath, std::ios::out);
                if (!keyFile.is_open())
                {
                    throw std::runtime_error(
                        "Failed to open key file: " + keyPath.string()
                    );
                }

                auto castedChar = reinterpret_cast<const char *>(keyBuffer);
                keyFile.write(castedChar, strlen(castedChar));

                keyFile.close();

                LOG_DEBUG("Private key saved to: " + keyPath.string());
            }

            {
                unsigned char crtBuffer[2000];
                tmpRetVar = mbedtls_x509write_crt_pem(
                    &crtCtx,
                    crtBuffer,
                    sizeof(crtBuffer),
                    mbedtls_ctr_drbg_random,
                    &ctrDrbg
                );
                if (tmpRetVar != 0)
                {
                    throw std::runtime_error(
                        "Failed to write crt into buffer. Error code: " +
                        std::to_string(tmpRetVar)
                    );
                }

                if (std::filesystem::exists(crtPath))
                {
                    std::filesystem::remove(crtPath);
                }

                std::ofstream crtFile(crtPath, std::ios::out);
                if (!crtFile.is_open())
                {
                    throw std::runtime_error(
                        "Failed to open crt file: " + crtPath.string()
                    );
                }

                auto castedChar = reinterpret_cast<const char *>(crtBuffer);
                crtFile.write(castedChar, strlen(castedChar));

                crtFile.close();

                LOG_DEBUG("Cert file saved to: " + crtPath.string());
            }
            LOG_INFO("Self-signed TLS cert successfully generated.");
        }
        catch (const std::exception &err)
        {
            LOG_ERROR(
                "Unexpected error during cert generation, error: {}", err.what()
            );
            cleanUp();
            return -1;
        }

        cleanUp();
        return 0;
    }

    bool initWsCert(std::filesystem::path &baseDir, bool force)
    {
        std::filesystem::path crtPath = baseDir / "wss.crt";
        std::filesystem::path keyPath = baseDir / "wss.key";

        bool isExists = checkCertExists(crtPath, keyPath);
        if (isExists && !force)
        {
            LOG_INFO(
                "WebSocket TLS cert already exists, skipping regeneration..."
            );
            return true;
        }
        LOG_INFO("Generating WebSocket TLS cert...");

        std::string certHost("localhost");
        std::string certIdentifier("wss");

        int result = genEC256TlsCert(baseDir, certHost, certIdentifier);

        if (result == 0)
        {
            LOG_INFO("Cert successfully generated.");
        }
        else
        {
            LOG_WARN("Failed to generate WebSocket TLS cert.");
        }

        if (force)
        {
            AikariLifecycle::Utils::Config::editConfig(
                [](auto &config)
                {
                    config->tls.regenWsCertNextLaunch = false;
                },
                true
            );

            return result == 0 ? true : false;
        }
    }
}  // namespace AikariUtils::SSLUtils
