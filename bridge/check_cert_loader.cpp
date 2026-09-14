#include "check_cert_loader.h"

#include "json.h"
#include "openssl.h"

#include <openssl/pem.h>
#include <openssl/pkcs12.h>
#include <openssl/provider.h>
#include <openssl/x509.h>

namespace {

static X509* LoadIssuerForCert(X509* cert)
{
	if (!cert) {
		return NULL;
	}

	unsigned long issuerHash = X509_issuer_name_hash(cert);
	const char* pem = ZSignAsset::WWDRIntermediatePEM(issuerHash);
	if (!pem) {
		return NULL;
	}

	BIO* bio = BIO_new_mem_buf(pem, (int)strlen(pem));
	if (!bio) {
		return NULL;
	}
	X509* issuer = PEM_read_bio_X509(bio, NULL, 0, NULL);
	BIO_free(bio);
	return issuer;
}

static X509* MatchCertFromProvision(const string& provData, EVP_PKEY* evpPKey)
{
	string strProvContent;
	if (!ZSignAsset::GetCMSContent(provData, strProvContent)) {
		return NULL;
	}

	jvalue jvProv;
	if (!jvProv.read_plist(strProvContent)) {
		return NULL;
	}

	for (size_t i = 0; i < jvProv["DeveloperCertificates"].size(); i++) {
		string strCertData = jvProv["DeveloperCertificates"][i].as_data();
		BIO* bioCert = BIO_new_mem_buf(strCertData.c_str(), (int)strCertData.size());
		if (!bioCert) {
			continue;
		}
		X509* x509Cert = d2i_X509_bio(bioCert, NULL);
		BIO_free(bioCert);
		if (!x509Cert) {
			continue;
		}
		if (X509_check_private_key(x509Cert, evpPKey)) {
			return x509Cert;
		}
		X509_free(x509Cert);
	}
	return NULL;
}

} // namespace

bool LoadCheckCertAssets(
	const string& strPKeyFile,
	const string& strProvFile,
	const string& strPassword,
	CheckCertLoaded& out,
	string& outError)
{
	out.cert = NULL;
	out.issuer = NULL;
	outError.clear();
	string provData;
	if (!ZFile::ReadFile(strProvFile.c_str(), provData) || provData.empty()) {
		outError = "Can't find provision file.";
		return false;
	}

	EVP_PKEY* evpPKey = NULL;
	X509* x509Cert = NULL;
	BIO* bioPKey = BIO_new_file(strPKeyFile.c_str(), "rb");
	if (!bioPKey) {
		outError = "Can't load p12 or private key file. Please input the correct file and password.";
		return false;
	}

	evpPKey = PEM_read_bio_PrivateKey(bioPKey, NULL, NULL, (void*)strPassword.c_str());
	if (!evpPKey) {
		BIO_reset(bioPKey);
		evpPKey = d2i_PrivateKey_bio(bioPKey, NULL);
	}
	if (!evpPKey) {
		BIO_reset(bioPKey);
		OSSL_PROVIDER_load(NULL, "legacy");
		PKCS12* p12 = d2i_PKCS12_bio(bioPKey, NULL);
		if (p12) {
			STACK_OF(X509)* caCerts = NULL;
			if (PKCS12_parse(p12, strPassword.c_str(), &evpPKey, &x509Cert, &caCerts) != 1) {
				if (caCerts) {
					sk_X509_pop_free(caCerts, X509_free);
				}
			} else if (caCerts) {
				sk_X509_pop_free(caCerts, X509_free);
			}
			PKCS12_free(p12);
		}
	}
	BIO_free(bioPKey);

	if (!evpPKey) {
		outError = "Unable to initialize certificate. Please check your password.";
		if (x509Cert) {
			X509_free(x509Cert);
		}
		return false;
	}

	if (!x509Cert) {
		x509Cert = MatchCertFromProvision(provData, evpPKey);
	}
	EVP_PKEY_free(evpPKey);

	if (!x509Cert) {
		outError = "Unable to initialize certificate. Please check your password.";
		return false;
	}

	X509* issuer = LoadIssuerForCert(x509Cert);
	if (!issuer) {
		X509_free(x509Cert);
		outError = "Unable to determine issuer of the certificate. It is signed by Apple Developer?";
		return false;
	}

	out.cert = x509Cert;
	out.issuer = issuer;
	return true;
}

void FreeCheckCertLoaded(CheckCertLoaded& loaded)
{
	if (loaded.cert) {
		X509_free((X509*)loaded.cert);
		loaded.cert = NULL;
	}
	if (loaded.issuer) {
		X509_free((X509*)loaded.issuer);
		loaded.issuer = NULL;
	}
}
