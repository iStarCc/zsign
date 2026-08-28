#pragma once

#include "common.h"

#include "archive_cancel.h"

class Zip
{
public:
	static bool Archive(const string& strFolder, const string& strZipFile, int nZipLevel);
	/** 仅将 `…/Payload` 目录树打入 zip，条目前缀恒为 `Payload/…`。 */
	static bool ArchivePayloadFolderForIPA(const string& strPayloadFolder, const string& strZipFile, int nZipLevel);
	static bool Extract(const char* zip_file, const char* output_folder);
	/** 解压 ZIP/IPA 到目录；与 `Extract` 相同清理/安全路径规则，额外输出与压缩一致的条目进度与大文件心跳（`ZipLogExtractUnified`）。 */
	static bool ExtractWithProgress(const char* zip_file, const char* output_folder);
	/** 校验 zip 是否为 IPA 布局：至少存在 `Payload/xxx.app` 路径。 */
	static bool HasIpaLayout(const char* zip_file);
	/** 解压到已有目录：不删除输出根目录。 */
	static bool ExtractWithProgressIntoExisting(const char* zip_file, const char* output_folder);

private:
	static bool _ReadFileFromZip(
		void* hZip,
		const string& strPath,
		const string& strRootFolder,
		int entriesCompletedBefore,
		int entryTotal,
		const string& strRelativePathForLog,
		uint64_t uncompressedSize,
		bool withProgress);
	static bool _ExtractImpl(const char* zip_file, const char* output_folder, bool withProgress);
	static bool _WriteFileToZip(void* hZip, const string& strFile, const string& strRelativePath, int zip_level, int completedEntriesBefore, int entryTotal);
	static bool _CreateFolderToZip(void* hZip, const string& strFolder, const string& strRelativePath, int zip_level);
	static void GetModificationTime(const char* path, void* zi);
};
