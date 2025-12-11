#pragma once 
#include "vmime/security/cert/defaultCertificateVerifier.hpp"
#include "vmime/security/cert/certificateVerifier.hpp"
#include <string>
#include <vector>
#include <memory>

using CertificateVerifier = std::shared_ptr<vmime::security::cert::certificateVerifier>;
struct FileCertificateVerifier : vmime::security::cert::certificateVerifier {
  FileCertificateVerifier(
    std::vector<std::string> certPaths
  );
private:
  vmime::security::cert::defaultCertificateVerifier base;
};
