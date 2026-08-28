#include "zsign.hpp"

#include "check_cert_loader.h"

#include "common.h"
#include "timer.h"

#include <openssl/ocsp.h>
#include <openssl/x509.h>
#include <openssl/asn1.h>

#include <string>

extern "C" {

int checkCert(
	NSString* prov,
	NSString* key,
	NSString* pass,
	void (^completionHandler)(int status, NSDate* _Nullable expirationDate, NSString* _Nullable error))
{
	ZTimer gtimer;

	if (!key || !prov || !pass) {
		completionHandler(2, nil, @"One or more required paths or password is missing.");
		return -1;
	}

	string strPKeyFile = [key cStringUsingEncoding:NSUTF8StringEncoding];
	string strProvFile = [prov cStringUsingEncoding:NSUTF8StringEncoding];
	string strPassword = [pass cStringUsingEncoding:NSUTF8StringEncoding];

	__block CheckCertLoaded loaded = {NULL, NULL};
	string loadError;
	if (!LoadCheckCertAssets(strPKeyFile, strProvFile, strPassword, loaded, loadError)) {
		NSString* msg = loadError.empty()
			? @"Unable to initialize certificate. Please check your password."
			: [NSString stringWithUTF8String:loadError.c_str()];
		completionHandler(2, nil, msg);
		return -1;
	}

	X509* cert = (X509*)loaded.cert;
	X509* issuer = (X509*)loaded.issuer;

	STACK_OF(ACCESS_DESCRIPTION)* aia = (STACK_OF(ACCESS_DESCRIPTION)*)X509_get_ext_d2i(cert, NID_info_access, 0, 0);
	if (!aia) {
		FreeCheckCertLoaded(loaded);
		completionHandler(2, nil, @"No AIA (OCSP) extension found in certificate");
		return -5;
	}

	ASN1_IA5STRING* uri = nullptr;
	for (int i = 0; i < sk_ACCESS_DESCRIPTION_num(aia); i++) {
		ACCESS_DESCRIPTION* ad = sk_ACCESS_DESCRIPTION_value(aia, i);
		if (OBJ_obj2nid(ad->method) == NID_ad_OCSP && ad->location->type == GEN_URI) {
			uri = ad->location->d.uniformResourceIdentifier;
			break;
		}
	}

	if (!uri) {
		sk_ACCESS_DESCRIPTION_pop_free(aia, ACCESS_DESCRIPTION_free);
		FreeCheckCertLoaded(loaded);
		completionHandler(2, nil, @"No OCSP URI found in certificate.");
		return -6;
	}

	OCSP_REQUEST* req = OCSP_REQUEST_new();
	OCSP_CERTID* cert_id = OCSP_cert_to_id(nullptr, cert, issuer);
	OCSP_request_add0_id(req, cert_id);
	unsigned char* der = 0;
	int len = i2d_OCSP_REQUEST(req, &der);

	NSMutableURLRequest* request = [NSMutableURLRequest requestWithURL:[NSURL URLWithString:[NSString stringWithUTF8String:(const char*)uri->data]]];
	[request setHTTPMethod:@"POST"];
	[request setHTTPBody:[NSData dataWithBytes:der length:(NSUInteger)len]];
	[request setValue:@"application/ocsp-request" forHTTPHeaderField:@"Content-Type"];
	[request setValue:@"application/ocsp-response" forHTTPHeaderField:@"Accept"];

	OPENSSL_free(der);
	sk_ACCESS_DESCRIPTION_pop_free(aia, ACCESS_DESCRIPTION_free);
	OCSP_REQUEST_free(req);

	NSURLSession* session = [NSURLSession sharedSession];
	NSURLSessionDataTask* task = [session dataTaskWithRequest:request
										 completionHandler:^(NSData* _Nullable data, NSURLResponse* _Nullable response, NSError* _Nullable error) {
		if (error) {
			FreeCheckCertLoaded(loaded);
			completionHandler(2, nil, error.localizedDescription);
			return;
		}

		NSHTTPURLResponse* httpResponse = (NSHTTPURLResponse*)response;
		if (httpResponse.statusCode == 200 && data) {
			const void* respBytes = [data bytes];
			OCSP_RESPONSE* resp = NULL;
			d2i_OCSP_RESPONSE(&resp, (const unsigned char**)&respBytes, (long)[data length]);
			OCSP_BASICRESP* basic = OCSP_response_get1_basic(resp);
			ASN1_TIME* expirationDateAsn1 = X509_get_notAfter(cert);
			NSString* fullDateString = [NSString stringWithFormat:@"20%s", expirationDateAsn1->data];

			NSDateFormatter* formatter = [[NSDateFormatter alloc] init];
			formatter.dateFormat = @"yyyyMMddHHmmss'Z'";
			formatter.timeZone = [NSTimeZone timeZoneWithAbbreviation:@"UTC"];
			formatter.locale = NSLocale.currentLocale;
			NSDate* expirationDate = [formatter dateFromString:fullDateString];

			int status = 0;
			int reason = 0;
			OCSP_CERTID* lookupId = OCSP_cert_to_id(nullptr, cert, issuer);
			if (OCSP_resp_find_status(basic, lookupId, &status, &reason, NULL, NULL, NULL)) {
				completionHandler(status, expirationDate, nil);
			} else {
				completionHandler(2, expirationDate, nil);
			}

			OCSP_CERTID_free(lookupId);
			OCSP_BASICRESP_free(basic);
			OCSP_RESPONSE_free(resp);
		} else {
			completionHandler(2, nil, @"Invalid response or no data");
		}
		FreeCheckCertLoaded(loaded);
	}];

	[task resume];
	(void)gtimer;
	return 1;
}

}
