#include "common.h"

#include "verify_archo.h"

#include "archo.h"
#include "sha.h"
#include "signing.h"

#include <cstring>

bool VerifyArchOCodeSlots(const ZArchO* archo, bool bSilent)
{
	if (NULL == archo || NULL == archo->m_pBase || NULL == archo->m_pSignBase
		|| archo->m_uSignLength < sizeof(CS_SuperBlob)) {
		return false;
	}

	CS_SuperBlob* psb = (CS_SuperBlob*)archo->m_pSignBase;
	if (CSMAGIC_EMBEDDED_SIGNATURE != LE(psb->magic)) {
		return false;
	}

	CS_BlobIndex* pbi = (CS_BlobIndex*)(archo->m_pSignBase + sizeof(CS_SuperBlob));
	for (uint32_t i = 0; i < LE(psb->count); i++, pbi++) {
		uint32_t slotType = LE(pbi->type);
		if (slotType != CSSLOT_CODEDIRECTORY && slotType != CSSLOT_ALTERNATE_CODEDIRECTORIES) {
			continue;
		}

		uint8_t* pSlotBase = archo->m_pSignBase + LE(pbi->offset);
		CS_CodeDirectory cdHeader = *((CS_CodeDirectory*)pSlotBase);
		if (CSMAGIC_CODEDIRECTORY != LE(cdHeader.magic)) {
			if (!bSilent) {
				ZLog::Error(">>> Verify: invalid CodeDirectory magic\n");
			}
			return false;
		}

		uint32_t uCodeLimit = LE(cdHeader.codeLimit);
		uint32_t uPageSize = 1u << cdHeader.pageSize;
		uint32_t nCodeSlots = LE(cdHeader.nCodeSlots);
		uint8_t hashSize = cdHeader.hashSize;
		uint8_t hashType = cdHeader.hashType;
		uint8_t* pHashes = pSlotBase + LE(cdHeader.hashOffset);

		for (uint32_t s = 0; s < nCodeSlots; s++) {
			uint32_t uOffset = uPageSize * s;
			uint32_t uSize = (uOffset + uPageSize <= uCodeLimit) ? uPageSize : (uCodeLimit - uOffset);

			string strComputed;
			if (1 == hashType) {
				ZSHA::SHA1(archo->m_pBase + uOffset, uSize, strComputed);
			} else if (2 == hashType) {
				ZSHA::SHA256(archo->m_pBase + uOffset, uSize, strComputed);
			} else {
				if (!bSilent) {
					ZLog::ErrorV(">>> Verify: unknown hashType %u\n", hashType);
				}
				return false;
			}

			if (strComputed.size() < hashSize
				|| 0 != memcmp(strComputed.data(), pHashes + hashSize * s, hashSize)) {
				if (!bSilent) {
					ZLog::ErrorV(">>> Verify: code slot %u hash mismatch (hashType=%u)\n", s, hashType);
				}
				return false;
			}
		}
	}
	return true;
}

bool VerifyArchOEmbeddedSignature(const ZArchO* archo, bool bCheckCMS)
{
	if (NULL == archo || NULL == archo->m_pBase || NULL == archo->m_pSignBase
		|| archo->m_uSignLength < sizeof(CS_SuperBlob)) {
		ZLog::Error(">>> Verify: no embedded signature found\n");
		return false;
	}

	CS_SuperBlob* psb = (CS_SuperBlob*)archo->m_pSignBase;
	if (CSMAGIC_EMBEDDED_SIGNATURE != LE(psb->magic)) {
		ZLog::Error(">>> Verify: invalid SuperBlob magic\n");
		return false;
	}

	uint32_t uBlobLength = LE(psb->length);
	if (uBlobLength > archo->m_uSignLength) {
		ZLog::Error(">>> Verify: SuperBlob length exceeds signature region\n");
		return false;
	}

	bool bHasCodeDir = false;
	bool bHasCMS = false;
	CS_BlobIndex* pbi = (CS_BlobIndex*)(archo->m_pSignBase + sizeof(CS_SuperBlob));
	for (uint32_t i = 0; i < LE(psb->count); i++, pbi++) {
		uint32_t slotType = LE(pbi->type);
		uint8_t* pSlotBase = archo->m_pSignBase + LE(pbi->offset);

		if (slotType == CSSLOT_CODEDIRECTORY || slotType == CSSLOT_ALTERNATE_CODEDIRECTORIES) {
			CS_CodeDirectory* pcd = (CS_CodeDirectory*)pSlotBase;
			if (CSMAGIC_CODEDIRECTORY != LE(pcd->magic)) {
				ZLog::Error(">>> Verify: CodeDirectory has invalid magic\n");
				return false;
			}
			if (LE(pcd->codeLimit) != archo->m_uCodeLength) {
				ZLog::ErrorV(
					">>> Verify: CodeDirectory codeLimit(%u) != actual codeLength(%u)\n",
					LE(pcd->codeLimit),
					archo->m_uCodeLength);
				return false;
			}
			bHasCodeDir = true;
		} else if (slotType == CSSLOT_SIGNATURESLOT) {
			uint32_t* pMagic = (uint32_t*)pSlotBase;
			if (CSMAGIC_BLOBWRAPPER != LE(*pMagic)) {
				ZLog::Error(">>> Verify: CMS blob has invalid magic\n");
				return false;
			}
			bHasCMS = true;
		}
	}

	if (!bHasCodeDir) {
		ZLog::Error(">>> Verify: missing CodeDirectory\n");
		return false;
	}

	if (bCheckCMS && !bHasCMS) {
		ZLog::Error(">>> Verify: missing CMS Signature\n");
		return false;
	}

	return true;
}
