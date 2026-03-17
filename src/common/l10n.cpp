//
//  l10n.cpp
//  zsign
//

#include "l10n.h"
#include "log_keys.h"
#include "common.h"

using namespace ZL10nKeys;

std::string ZL10n::g_locale = "en";
std::map<std::string, std::map<std::string, std::string>> ZL10n::g_strings;

void ZL10n::SetLocale(const char* locale) {
	if (locale && strlen(locale) >= 2) {
		g_locale = locale;
		if (g_locale != "zh" && g_locale != "en") {
			g_locale = "en";
		}
	}
}

const char* ZL10n::GetLocale() {
	return g_locale.c_str();
}

void ZL10n::InitStrings() {
	if (!g_strings.empty()) return;
	
	// 简体中文
	map<string, string>& zh = g_strings["zh"];
	zh[FAILED_RESOLVE_EXEC_PATH] = "➤ 无法解析可执行文件路径: %s\n";
	zh[INVALID_MACHO_PATH] = "➤ 无效的 Mach-O 文件: %s\n";
	zh[MODIFIED_CONTENT_AT] = "➤ 修改后的内容位于: %s（从 .ipa 解压，如需可手动重新打包）\n";
	zh[DYLIBS_IN] = "➤ %s 中的 Dylibs:\n";
	zh[MACHO_SIGNED] = "➤ MachO 已签名!\n";
	zh[MACHO_NOT_SIGNED] = "➤ MachO 未签名。\n";
	zh[DYLIB_INJECTED_OK] = "➤ Dylib 注入成功!\n";
	zh[DYLIB_INJECT_FAILED] = "➤ Dylib 注入失败。\n";
	zh[DYLIBS_UNINSTALLED_OK] = "➤ Dylibs 已成功移除!\n";
	zh[NO_DYLIBS_FOUND] = "➤ Mach-O 文件中未发现 Dylibs。\n";
	zh[DYLIB_PATH_CHANGED_OK] = "➤ Dylib 路径修改成功!\n";
	zh[DYLIB_PATH_CHANGE_FAILED] = "➤ Dylib 路径修改失败。\n";
	zh[INVALID_PATH] = "➤ 路径无效! %s\n";
	zh[INVALID_INPUT_PATH] = "➤ 输入路径无效! %s\n";
	zh[INVALID_TEMP_FOLDER] = "➤ 临时目录无效! %s\n";
	zh[OUTPUT_PATH_REQUIRED] = "➤ signIPA 必须指定输出路径。\n";
	zh[SIGNING] = "➤ 正在签名: %s %s\n";
	zh[SIGNED_FMT] = "➤ 签名完成: %s\n";
	zh[SIGNED_OK] = "签名成功";
	zh[SIGNED_FAILED] = "签名失败";
	zh[DONE] = "➤ 处理完成。\n";
	zh[UNZIP] = "➤ 正在解压: %s (%s) -> %s ... \n";
	zh[UNZIP_FAILED] = "➤ 解压失败!\n";
	zh[UNZIP_OK] = "➤ 解压完成!\n";
	zh[ARCHIVING] = "➤ 正在打包: %s ... \n";
	zh[ARCHIVE_OK] = "➤ 打包完成 (%s)\n";
	zh[ARCHIVE_PROGRESS] = "➤ 压缩中: %zu/%zu (%d%%)\n";
	zh[ARCHIVE_FAILED] = "➤ 打包失败!\n";
	zh[FAILED_CREATE_PAYLOAD] = "➤ 创建 Payload 目录失败!\n";
	zh[FAILED_COPY_APP] = "➤ 复制应用到 Payload 目录失败: %s\n";
	zh[REMOVED_EMBEDDED_PROVISION] = "➤ 已移除 embedded.mobileprovision\n";
	zh[SIGN_FILE] = "➤ 签名文件: %s\n";
	zh[SKIP_NON_MACHO] = "➤ 警告: 跳过非 Mach-O 文件: %s\n";
	zh[CANT_GET_BUNDLE_INFO] = "➤ 无法从 Info.plist 获取标识符(BundleId)、可执行文件(BundleExecutable) 或 SHASum: %s\n";
	zh[CANT_PARSE_EXECUTABLE] = "➤ 无法解析可执行文件: %s\n";
	zh[CREATE_CODERESOURCES_FAILED] = "➤ 创建 CodeResources 失败: %s\n";
	zh[CANT_GET_SHASUM] = "➤ 无法获取变更文件的 SHASum: %s\n";
	zh[WRITING_CODERESOURCES_FAILED] = "➤ 写入 CodeResources 失败: %s\n";
	zh[CANT_WRITE_PROVISION] = "➤ 无法写入 embedded.mobileprovision!\n";
	zh[CANT_FIND_PLUGIN_INFO] = "➤ 找不到插件的 Info.plist: %s\n";
	zh[BUNDLE_ID] = "➤ 标识符(BundleId): %s -> %s\n";
	zh[BUNDLE_ID_VALUE] = "➤ 标识符(BundleId): %s\n";
	zh[BUNDLE_ID_PLUGIN] = "➤ 标识符(BundleId): %s -> %s (Plugin)\n";
	zh[BUNDLE_ID_WK] = "➤ 标识符(BundleId): %s -> %s (Plugin-WKCompanionAppBundleIdentifier)\n";
	zh[BUNDLE_ID_EXT] = "➤ 标识符(BundleId): %s -> %s (NSExtension-NSExtensionAttributes-WKAppBundleIdentifier)\n";
	zh[CANT_FIND_APP_INFO] = "➤ 找不到应用的 Info.plist: %s\n";
	zh[BUNDLE_NAME] = "➤ 名称(BundleName): %s -> %s\n";
	zh[BUNDLE_VERSION] = "➤ 版本(BundleVersion): %s -> %s\n";
	zh[ENABLED_DOCUMENTS] = "➤ 已启用文档共享支持\n";
	zh[MIN_OS_VERSION] = "➤ 最低系统版本(MinimumOSVersion): %s -> %s\n";
	zh[REMOVED] = "➤ 已移除 %s\n";
	zh[REMOVED_UI_DEVICES] = "➤ 已移除支持的设备(UISupportedDevices)\n";
	zh[CANT_FIND_APP_FOLDER] = "➤ 找不到应用目录: %s\n";
	zh[CANT_GET_INFO_PLIST] = "➤ 无法从 Info.plist 获取标识符(BundleId)、版本(BundleVersion)或可执行文件(BundleExecutable): %s\n";
	zh[SIGN_FOLDER] = "➤ 签名目录: %s (%s)\n";
	zh[SIGNING_APP] = "➤ 正在签名: %s ...\n";
	zh[APP_NAME] = "➤ 应用名称(AppName): %s\n";
	zh[VERSION] = "➤ 版本(Version): %s\n";
	zh[TEAM_ID] = "➤ 团队ID(TeamId): %s\n";
	zh[SUBJECT_CN] = "➤ 证书主体(SubjectCN): %s\n";
	zh[READ_CACHE] = "➤ 读取缓存(ReadCache): %s\n";
	zh[CACHE_YES] = "是";
	zh[CACHE_NO] = "否";
	zh[ADHOC] = " (Ad-hoc)";
	zh[DEBUG_OPTION] = "➤ 选项:\t-%c, %s\n";
	zh[DEBUG_ARGUMENT] = "➤ 参数:\t%s\n";
	
	zh[INVALID_ZIP_LEVEL] = "➤ 无效的压缩级别! 请输入 0 - 9。\n";
	zh[CANT_FIND_PAYLOAD] = "➤ 找不到 Payload 目录!\n";
	zh[FAILED_INIT_PROVISION] = "➤ 初始化 provision 失败: %s\n";
	
	// timer.cpp 耗时格式
	zh[TIME_FMT_S] = "%.3f秒";
	zh[TIME_FMT_M_S] = "%llu分 %.3f秒";
	zh[TIME_FMT_H_S] = "%llu小时 %.3f秒";
	zh[TIME_FMT_H_M_S] = "%llu小时 %llu分 %.3f秒";
	zh[TIME_FMT_WRAP] = "%s (%s, %lluus)\n";

	// openssl.cpp
	zh[NCONF_NEW_FAILED] = "➤ NCONF_new 创建失败\n";
	zh[NCONF_LOAD_BIO_FAILED] = "➤ NCONF_load_bio 加载失败 %d\n";
	zh[NCONF_GET_STRING_FAILED] = "➤ NCONF_get_string 获取失败\n";
	zh[UNKNOWN_ISSUER_HASH] = "➤ 未知的 issuer hash!\n";
	zh[CANT_READ_ENTITLEMENTS_FILE] = "➤ 无法读取 entitlements 文件!\n";
	zh[CANT_FIND_PROVISION_FILE] = "➤ 找不到 provision 文件!\n";
	zh[CANT_FIND_TEAM_ID] = "➤ 找不到 TeamId!\n";
	zh[CANT_LOAD_P12_OR_KEY] = "➤ 无法加载 p12 或私钥文件，请检查文件路径和密码!\n";
	zh[CANT_FIND_PAIRED_CERT_KEY] = "➤ 找不到匹配的证书和私钥!\n";
	zh[CANT_FIND_CERT_SUBJECT_CN] = "➤ 找不到证书主题 CN!\n";

	// macho.cpp
	zh[INVALID_ARCH_IN_FAT] = "➤ fat Mach-O 文件中的架构无效!\n";
	zh[INVALID_MACHO_FILE] = "➤ 无效的 Mach-O 文件!\n";
	zh[INVALID_MACHO_MAGIC] = "➤ 无效的 Mach-O 文件 (magic: 0x%08x)!\n";
	zh[CODESIGN_MUNMAP_FAILED] = "➤ CodeSign 写入 (munmap) 失败! 错误: %p, %lu, %s\n";
	zh[REALLOC_CODESIGN_SPACE] = "➤ 重新分配 CodeSignature 空间...\n";
	zh[REALLOC_CODESIGN_SPACE_SUCCESS] = "➤ CodeSignature 空间分配成功\n";
	zh[REALLOC_CODESIGN_SPACE_FAILED] = "➤ CodeSignature 空间分配失败\n";
	zh[FAILED] = "➤ 失败!\n";
	zh[SUCCESS] = "➤ 成功!\n";
	zh[INJECT_DYLIB] = "➤ 注入 Dylib: %s %s...\n";
	zh[INJECT_WEAK] = "(weak)";
	zh[CHANGE_DYLIB_PATH] = "➤ 修改 Dylib 路径: %s -> %s ...\n";
	zh[FAILED_CHANGE_PATH_ARCH] = "➤ 在某个架构中修改路径失败!\n";
	zh[SUCCESS_CHANGED_DYLIB_PATHS] = "➤ 所有 Dylib 路径修改成功!\n";
	zh[FOUND_DYLIBS] = "➤ 发现 %zu 个 Dylib:\n";

	// archo.cpp
	zh[FILE_NOT_SIGNED] = "文件未签名。\n";
	zh[FILE_SIGNED] = "文件已签名。\n";
	zh[MACHO_INFO] = "➤ Mach-O 信息:\n";
	zh[EMBEDDED_INFO_PLIST] = "\n➤ 内嵌 Info.plist:\n";
	zh[CANT_FIND_CODESIGN_SEGMENT] = "➤ 找不到 CodeSignature 段!\n";
	zh[BUILD_CODESIGN_FAILED] = "➤ 构建 CodeSignature 失败!\n";
	zh[NO_ENOUGH_CODESIGN_SPACE] = "➤ CodeSignature 空间不足 (当前: %d，需要: %d)。\n";
	zh[CANT_FIND_LOADCMD_SPACE_CODESIGN] = "➤ 找不到可用于 CodeSignature 的 LoadCommands 空间!\n";
	zh[CANT_FIND_LOADCMD_SPACE_DYLIB] = "➤ 找不到可用于 LC_LOAD_DYLIB 或 LC_LOAD_WEAK_DYLIB 的 LoadCommands 空间!\n";
	zh[INSUFFICIENT_SPACE_UPDATE_DYLIB] = "➤ 更新 Dylib 路径空间不足!\n";
	zh[DYLIB_PATH_CHANGED] = "➤ Dylib 路径已修改: %s -> %s\n";
	zh[OLD_DYLIB_PATH_NOT_FOUND] = "➤ 未找到原始 Dylib 路径: %s\n";
	zh[LOAD_TYPE_CHANGED] = "➤    %s -> %s\n";
	
	zh[DYLIB_LIST_ITEM] = "\t%s\n";
	zh[MACHO_SEPARATOR] = "------------------------------------------------------------------\n";
	zh[MACHO_FILETYPE] = "\t文件类型: \t%s\n";
	zh[MACHO_TOTALSIZE] = "\t总大小: \t%u (%s)\n";
	zh[MACHO_PLATFORM] = "\t平台: \t%u\n";
	zh[MACHO_CPUARCH] = "\tCPU架构: \t%s\n";
	zh[MACHO_CPUTYPE] = "\tCPU类型: \t0x%x\n";
	zh[MACHO_CPUSUBTYPE] = "\tCPU子类型: \t0x%x\n";
	zh[MACHO_BIGENDIAN] = "\t大端序: \t%d\n";
	zh[MACHO_ENCRYPTED] = "\t加密: \t%d\n";
	zh[MACHO_CMDCOUNT] = "\t命令数: \t%d\n";
	zh[MACHO_CODELENGTH] = "\t代码长度: \t%d (%s)\n";
	zh[MACHO_SIGNLENGTH] = "\t签名长度: \t%d (%s)\n";
	zh[MACHO_SPARELENGTH] = "\t剩余长度: \t%d (%s)\n";
	zh[MACHO_MIN_IPHONEOS] = "\tMIN_IPHONEOS: \t0x%x\n";
	zh[MACHO_LC_RPATH] = "\tLC_RPATH: \t%s\n";
	zh[MACHO_LC_LOAD_DYLIB] = "\tLC_LOAD_DYLIB: \n";
	zh[MACHO_LC_LOAD_WEAK_DYLIB] = "\tLC_LOAD_WEAK_DYLIB: \n";
	zh[MACHO_DYLIB_ITEM] = "\t\t\t%s\n";
	zh[MACHO_DYLIB_WEAK_ITEM] = "\t\t\t%s (weak)\n";
	zh[MACHO_PLIST_LENGTH] = "\t长度: \t%lu\n";
	zh[MACHO_PLIST_CONTENT] = "\t内容: \t%s\n";
	zh[MACHO_DYLIB_CLEAR] = "\t\t\t%s\t已清除\n";
	
	zh[CODESIGN_SEGMENT_HEADER] = "\n➤ CodeSignature 段: \n";
	zh[CODESIGN_MAGIC] = "\tmagic: \t\t0x%x\n";
	zh[CODESIGN_LENGTH] = "\tlength: \t%d\n";
	zh[CODESIGN_SLOTS] = "\tslots: \t\t%d\n";
	zh[CODESIGN_SLOT_HEADER] = "\n  > %s: \n";
	zh[CODESIGN_SLOT_TYPE] = "\ttype: \t\t0x%x\n";
	zh[CODESIGN_SLOT_OFFSET] = "\toffset: \t%u\n";
	zh[CODESIGN_SLOT_MAGIC] = "\tmagic: \t\t0x%x\n";
	zh[CODESIGN_SLOT_LENGTH] = "\tlength: \t%u\n";
	zh[CODESIGN_ENTITLEMENTS] = "\tentitlements: \n%s\n";
	zh[CODESIGN_VERSION] = "\tversion: \t0x%x\n";
	zh[CODESIGN_FLAGS] = "\tflags: \t\t%u\n";
	zh[CODESIGN_HASH_OFFSET] = "\thashOffset: \t%u\n";
	zh[CODESIGN_IDENT_OFFSET] = "\tidentOffset: \t%u\n";
	zh[CODESIGN_N_SPECIAL_SLOTS] = "\tnSpecialSlots: \t%u\n";
	zh[CODESIGN_N_CODE_SLOTS] = "\tnCodeSlots: \t%u\n";
	zh[CODESIGN_CODE_LIMIT] = "\tcodeLimit: \t%u\n";
	zh[CODESIGN_HASH_SIZE] = "\thashSize: \t%u\n";
	zh[CODESIGN_HASH_TYPE] = "\thashType: \t%u\n";
	zh[CODESIGN_SPARE1] = "\tspare1: \t%u\n";
	zh[CODESIGN_PAGE_SIZE] = "\tpageSize: \t%u\n";
	zh[CODESIGN_SPARE2] = "\tspare2: \t%u\n";
	zh[CODESIGN_SCATTER_OFFSET] = "\tscatterOffset: \t%u\n";
	zh[CODESIGN_TEAM_OFFSET] = "\tteamOffset: \t%u\n";
	zh[CODESIGN_SPARE3] = "\tspare3: \t%u\n";
	zh[CODESIGN_CODE_LIMIT64] = "\tcodeLimit64: \t%llu\n";
	zh[CODESIGN_EXEC_SEG_BASE] = "\texecSegBase: \t%llu\n";
	zh[CODESIGN_EXEC_SEG_LIMIT] = "\texecSegLimit: \t%llu\n";
	zh[CODESIGN_EXEC_SEG_FLAGS] = "\texecSegFlags: \t%llu\n";
	zh[CODESIGN_IDENTIFIER] = "\tidentifier: \t%s\n";
	zh[CODESIGN_TEAMID] = "\tteamid: \t%s\n";
	zh[CODESIGN_SPECIAL_SLOTS] = "\tSpecialSlots:\n";
	zh[CODESIGN_CODE_SLOTS] = "\tCodeSlots:\n";
	zh[CODESIGN_CODE_SLOTS_OMITTED] = "\tCodeSlots: \tomitted. (use -d option for details)\n";
	zh[CODESIGN_CERTIFICATES] = "\tCertificates: \n";
	zh[CODESIGN_SIGNED_ATTRS] = "\tSignedAttrs: \n";
	zh[CODESIGN_CERT_ENTRY] = "\t\t\t%s\t<=\t%s\n";
	zh[CODESIGN_ATTR_CONTENT_TYPE] = "\t  ContentType: \t%s => %s\n";
	zh[CODESIGN_ATTR_SIGNING_TIME] = "\t  SigningTime: \t%s => %s\n";
	zh[CODESIGN_ATTR_MSG_DIGEST] = "\t  MsgDigest: \t%s => %s\n";
	zh[CODESIGN_ATTR_CDHASHES] = "\t  CDHashes: \t%s => \n\t\t\t\t%s\n";
	zh[CODESIGN_ATTR_CDHASHES2] = "\t  CDHashes2: \t%s => \n";
	zh[CODESIGN_ATTR_CDHASHES2_ITEM] = "\t\t\t\t%s\n";
	zh[CODESIGN_ATTR_UNKNOWN] = "\t  UnknownAttr: \t%s => %s, type: %d, count: %d\n";
	zh[CODESIGN_NEWLINE] = "\n";

	// archive.cpp
	zh[ZIP_FAILED_OPEN_FILE] = "➤ Zip: 无法打开文件: %s\n";
	zh[ZIP_FAILED_ADD_FILE] = "➤ Zip: 无法添加文件到 Zip: %s\n";
	zh[ZIP_FAILED_CREATE_FOLDER] = "➤ Zip: 无法在 Zip 中创建文件夹: %s\n";
	zh[ZIP_INVALID_COMPRESSION_LEVEL] = "➤ Zip: 无效的压缩级别: %d\n";
	zh[ZIP_FAILED_CREATE] = "➤ Zip: 无法创建 Zip 文件: %s\n";

	// fs.cpp
	zh[WRITEFILE_FOPEN_FAILED] = "➤ WriteFile: fopen 失败! %s, %s\n";
	zh[APPENDFILE_FOPEN_FAILED] = "➤ AppendFile: fopen 失败! %s, %s\n";

	// util.cpp
	zh[SYSTEM_EXEC_ERROR] = "➤ SystemExec: \"%s\" 执行失败!\n";
	
	// metadata.cpp
	zh[GETMETADATA_CANT_READ] = "➤ GetMetadata: 无法读取 %s\n";
	zh[GETMETADATA_CANT_WRITE] = "➤ GetMetadata: 无法写入 %s\n";
	zh[METADATA_PATH] = "➤ Metadata: %s\n";
	
	// certcheck.cpp
	zh[CERT_SIGNED] = "➤ 已签名: %s\n";
	zh[CERT_NAME] = "➤ 名称: %s\n";
	zh[CERT_TYPE] = "➤ 类型: %s\n";
	zh[CERT_ORG] = "➤ 组织: %s\n";
	zh[CERT_TEAM] = "➤ 团队: %s\n";
	zh[CERT_SERIAL] = "➤ 序列号: %s\n";
	zh[CERT_ISSUED] = "➤ 签发时间: %s\n";
	zh[CERT_EXPIRES_EXPIRED] = "➤ 过期时间: %s (已过期 %d 天)\n";
	zh[CERT_EXPIRES_REMAINING_WARN] = "➤ 过期时间: %s (%d 天剩余!)\n";
	zh[CERT_EXPIRES_REMAINING] = "➤ 过期时间: %s (%d 天剩余)\n";
	zh[CERT_ALGORITHM] = "➤ 算法: %s\n";
	zh[CERT_ISSUER] = "➤ 签发者: %s\n";
	zh[OCSP_VALID] = "➤ OCSP: 有效 (ocsp.apple.com)\n";
	zh[OCSP_REVOKED] = "➤ OCSP: 已吊销\n";
	zh[OCSP_REVOKED_TIME] = "➤ 吊销时间: %s\n";
	zh[OCSP_UNKNOWN] = "➤ OCSP: 未知\n";
	zh[OCSP_ERROR] = "➤ OCSP: 错误\n";
	zh[OCSP_DETAIL] = "➤ 详情: %s\n";
	zh[CERT_CANT_READ_FILE] = "➤ 无法读取文件: %s\n";
	zh[CERT_UNKNOWN_FILE_TYPE] = "➤ 未知文件类型: %s\n";
	zh[CERT_CHECK] = "\n➤ 检查: %s (%s)\n";
	zh[CERT_SIGNED_NO] = "➤ 已签名: 否\n\n";
	zh[CERT_LOAD_FAILED] = "➤ 无法从 %s 加载证书\n";
	zh[OCSP_SKIPPED] = "➤ OCSP: 已跳过 (非 WWDR 签发者)\n";
	zh[CERT_YES] = "是";
	zh[CERT_NO] = "否";
	
	// English
	map<string, string>& en = g_strings["en"];
	en[FAILED_RESOLVE_EXEC_PATH] = "➤ Failed to resolve executable path: %s\n";
	en[INVALID_MACHO_PATH] = "➤ Invalid Mach-O file: %s\n";
	en[MODIFIED_CONTENT_AT] = "➤ Modified content at: %s (extracted from .ipa, rezip manually if needed)\n";
	en[DYLIBS_IN] = "➤ Dylibs in %s:\n";
	en[MACHO_SIGNED] = "➤ MachO is signed!\n";
	en[MACHO_NOT_SIGNED] = "➤ MachO is not signed.\n";
	en[DYLIB_INJECTED_OK] = "➤ Dylib injected successfully!\n";
	en[DYLIB_INJECT_FAILED] = "➤ Failed to inject dylib.\n";
	en[DYLIBS_UNINSTALLED_OK] = "➤ Dylibs uninstalled successfully!\n";
	en[NO_DYLIBS_FOUND] = "➤ No dylibs found in the Mach-O file.\n";
	en[DYLIB_PATH_CHANGED_OK] = "➤ Dylib path changed successfully!\n";
	en[DYLIB_PATH_CHANGE_FAILED] = "➤ Failed to change dylib path.\n";
	en[INVALID_PATH] = "➤ Invalid path! %s\n";
	en[INVALID_INPUT_PATH] = "➤ Invalid input path! %s\n";
	en[INVALID_TEMP_FOLDER] = "➤ Invalid temp folder! %s\n";
	en[OUTPUT_PATH_REQUIRED] = "➤ Output path is required for signIPA.\n";
	en[SIGNING] = "➤ Signing:\t%s %s\n";
	en[SIGNED_FMT] = "➤ Signed %s!\n";
	en[SIGNED_OK] = "OK";
	en[SIGNED_FAILED] = "Failed";
	en[DONE] = "➤ Done.\n";
	en[UNZIP] = "➤ Unzip:\t%s (%s) -> %s ... \n";
	en[UNZIP_FAILED] = "➤ Unzip failed!\n";
	en[UNZIP_OK] = "➤ Unzip OK!\n";
	en[ARCHIVING] = "➤ Archiving: \t%s ... \n";
	en[ARCHIVE_OK] = "➤ Archive OK! (%s)\n";
	en[ARCHIVE_PROGRESS] = "➤ Compressing: %zu/%zu (%d%%)\n";
	en[ARCHIVE_FAILED] = "➤ Archive failed!\n";
	en[FAILED_CREATE_PAYLOAD] = "➤ Failed to create Payload folder!\n";
	en[FAILED_COPY_APP] = "➤ Failed to copy app to Payload: %s\n";
	en[REMOVED_EMBEDDED_PROVISION] = "➤ Removed embedded.mobileprovision\n";
	en[SIGN_FILE] = "➤ SignFile:  %s\n";
	en[SKIP_NON_MACHO] = "➤ Warning: Skipping non-Mach-O file:  %s\n";
	en[CANT_GET_BUNDLE_INFO] = "➤ Can't get BundleID or BundleExecute or Info.plist SHASum! %s\n";
	en[CANT_PARSE_EXECUTABLE] = "➤ Can't parse BundleExecute file! %s\n";
	en[CREATE_CODERESOURCES_FAILED] = "➤ Create CodeResources failed! %s\n";
	en[CANT_GET_SHASUM] = "➤ Can't get changed file SHASum! %s\n";
	en[WRITING_CODERESOURCES_FAILED] = " Writing CodeResources failed! %s\n";
	en[CANT_WRITE_PROVISION] = "➤ Can't write embedded.mobileprovision!\n";
	en[CANT_FIND_PLUGIN_INFO] = "➤ Can't find Plugin's Info.plist! %s\n";
	en[BUNDLE_ID] = "➤ BundleId:  %s -> %s\n";
	en[BUNDLE_ID_VALUE] = "➤ BundleId:  %s\n";
	en[BUNDLE_ID_PLUGIN] = "➤ BundleId:  %s -> %s, Plugin\n";
	en[BUNDLE_ID_WK] = "➤ BundleId:  %s -> %s, Plugin-WKCompanionAppBundleIdentifier\n";
	en[BUNDLE_ID_EXT] = "➤ BundleId:  %s -> %s, NSExtension-NSExtensionAttributes-WKAppBundleIdentifier\n";
	en[CANT_FIND_APP_INFO] = "➤ Can't find app's Info.plist! %s\n";
	en[BUNDLE_NAME] = "➤ BundleName: %s -> %s\n";
	en[BUNDLE_VERSION] = "➤ BundleVersion: %s -> %s\n";
	en[ENABLED_DOCUMENTS] = "➤ Enabled documents support\n";
	en[MIN_OS_VERSION] = "➤ MinimumOSVersion: %s -> %s\n";
	en[REMOVED] = "➤ Removed %s\n";
	en[REMOVED_UI_DEVICES] = "➤ Removed UISupportedDevices\n";
	en[CANT_FIND_APP_FOLDER] = "➤ Can't find app folder! %s\n";
	en[CANT_GET_INFO_PLIST] = "➤ Can't get BundleID, BundleVersion, or BundleExecute in Info.plist! %s\n";
	en[SIGN_FOLDER] = "➤ SignFolder: %s (%s)\n";
	en[SIGNING_APP] = "➤ Signing:  %s ...\n";
	en[APP_NAME] = "➤ AppName:  %s\n";
	en[VERSION] = "➤ Version:  %s\n";
	en[TEAM_ID] = "➤ TeamId:  %s\n";
	en[SUBJECT_CN] = "➤ SubjectCN:  %s\n";
	en[READ_CACHE] = "➤ ReadCache:  %s\n";
	en[CACHE_YES] = "YES";
	en[CACHE_NO] = "NO";
	en[ADHOC] = " (Ad-hoc)";
	en[DEBUG_OPTION] = "➤ Option:\t-%c, %s\n";
	en[DEBUG_ARGUMENT] = "➤ Argument:\t%s\n";
	
	en[INVALID_ZIP_LEVEL] = "➤ Invalid zip level! Please input 0 - 9.\n";
	en[CANT_FIND_PAYLOAD] = "➤ Can't find payload directory!\n";
	en[FAILED_INIT_PROVISION] = "➤ Failed to init provision: %s\n";
	
	// openssl.cpp
	en[NCONF_NEW_FAILED] = "➤ NCONF_new failed\n";
	en[NCONF_LOAD_BIO_FAILED] = "➤ NCONF_load_bio failed %d\n";
	en[NCONF_GET_STRING_FAILED] = "➤ NCONF_get_string failed\n";
	en[UNKNOWN_ISSUER_HASH] = "➤ Unknown issuer hash!\n";
	en[CANT_READ_ENTITLEMENTS_FILE] = "➤ Can't read entitlements file!\n";
	en[CANT_FIND_PROVISION_FILE] = "➤ Can't find provision file!\n";
	en[CANT_FIND_TEAM_ID] = "➤ Can't find TeamId!\n";
	en[CANT_LOAD_P12_OR_KEY] = "➤ Can't load p12 or private key file. Please input the correct file and password!\n";
	en[CANT_FIND_PAIRED_CERT_KEY] = "➤ Can't find paired certificate and private key!\n";
	en[CANT_FIND_CERT_SUBJECT_CN] = "➤ Can't find paired certificate subject common name!\n";
	
	// macho.cpp
	en[INVALID_ARCH_IN_FAT] = "➤ Invalid arch file in fat mach-o file!\n";
	en[INVALID_MACHO_FILE] = "➤ Invalid mach-o file!\n";
	en[INVALID_MACHO_MAGIC] = "➤ Invalid mach-o file (magic: 0x%08x)!\n";
	en[CODESIGN_MUNMAP_FAILED] = "➤ CodeSign write(munmap) failed! Error: %p, %lu, %s\n";
	en[REALLOC_CODESIGN_SPACE] = "➤ Realloc CodeSignature space... \n";
	en[REALLOC_CODESIGN_SPACE_SUCCESS] = "➤ CodeSignature space allocation successful\n";
	en[REALLOC_CODESIGN_SPACE_FAILED] = "➤ CodeSignature space allocation failed\n";
	en[FAILED] = "➤ Failed!\n";
	en[SUCCESS] = "➤ Success!\n";
	en[INJECT_DYLIB] = "➤ InjectDylib: %s %s... \n";
	en[INJECT_WEAK] = "(weak)";
	en[CHANGE_DYLIB_PATH] = "➤ Change DyLib Path: %s -> %s ... \n";
	en[FAILED_CHANGE_PATH_ARCH] = "➤ Failed to change path in one of the architectures!\n";
	en[SUCCESS_CHANGED_DYLIB_PATHS] = "➤ Successfully changed all dylib paths!\n";
	en[FOUND_DYLIBS] = "➤ Found %zu dylibs:\n";
	
	// archo.cpp
	en[FILE_NOT_SIGNED] = "File is not signed.\n";
	en[FILE_SIGNED] = "File is signed.\n";
	en[MACHO_INFO] = "➤ MachO Info: \n";
	en[EMBEDDED_INFO_PLIST] = "\n➤ Embedded Info.plist: \n";
	en[CANT_FIND_CODESIGN_SEGMENT] = "➤ Can't find CodeSignature segment!\n";
	en[BUILD_CODESIGN_FAILED] = "➤ Build CodeSignature failed!\n";
	en[NO_ENOUGH_CODESIGN_SPACE] = "➤ No enough CodeSignature space (now: %d, need: %d).\n";
	en[CANT_FIND_LOADCMD_SPACE_CODESIGN] = "➤ Can't find free space of LoadCommands for CodeSignature!\n";
	en[CANT_FIND_LOADCMD_SPACE_DYLIB] = "➤ Can't find free space of LoadCommands for LC_LOAD_DYLIB or LC_LOAD_WEAK_DYLIB!\n";
	en[INSUFFICIENT_SPACE_UPDATE_DYLIB] = "➤ Insufficient space to update dylib path!\n";
	en[DYLIB_PATH_CHANGED] = "➤ Dylib Path Changed: %s -> %s\n";
	en[OLD_DYLIB_PATH_NOT_FOUND] = "➤ Old Dylib Path Not Found: %s\n";
	en[LOAD_TYPE_CHANGED] = "➤    %s -> %s\n";
	
	en[DYLIB_LIST_ITEM] = "\t%s\n";
	en[MACHO_SEPARATOR] = "------------------------------------------------------------------\n";
	en[MACHO_FILETYPE] = "\tFileType: \t%s\n";
	en[MACHO_TOTALSIZE] = "\tTotalSize: \t%u (%s)\n";
	en[MACHO_PLATFORM] = "\tPlatform: \t%u\n";
	en[MACHO_CPUARCH] = "\tCPUArch: \t%s\n";
	en[MACHO_CPUTYPE] = "\tCPUType: \t0x%x\n";
	en[MACHO_CPUSUBTYPE] = "\tCPUSubType: \t0x%x\n";
	en[MACHO_BIGENDIAN] = "\tBigEndian: \t%d\n";
	en[MACHO_ENCRYPTED] = "\tEncrypted: \t%d\n";
	en[MACHO_CMDCOUNT] = "\tCommandCount: \t%d\n";
	en[MACHO_CODELENGTH] = "\tCodeLength: \t%d (%s)\n";
	en[MACHO_SIGNLENGTH] = "\tSignLength: \t%d (%s)\n";
	en[MACHO_SPARELENGTH] = "\tSpareLength: \t%d (%s)\n";
	en[MACHO_MIN_IPHONEOS] = "\tMIN_IPHONEOS: \t0x%x\n";
	en[MACHO_LC_RPATH] = "\tLC_RPATH: \t%s\n";
	en[MACHO_LC_LOAD_DYLIB] = "\tLC_LOAD_DYLIB: \n";
	en[MACHO_LC_LOAD_WEAK_DYLIB] = "\tLC_LOAD_WEAK_DYLIB: \n";
	en[MACHO_DYLIB_ITEM] = "\t\t\t%s\n";
	en[MACHO_DYLIB_WEAK_ITEM] = "\t\t\t%s (weak)\n";
	en[MACHO_PLIST_LENGTH] = "\tlength: \t%lu\n";
	en[MACHO_PLIST_CONTENT] = "\tcontent: \t%s\n";
	en[MACHO_DYLIB_CLEAR] = "\t\t\t%s\tclear\n";
	
	en[CODESIGN_SEGMENT_HEADER] = "\n➤ CodeSignature Segment: \n";
	en[CODESIGN_MAGIC] = "\tmagic: \t\t0x%x\n";
	en[CODESIGN_LENGTH] = "\tlength: \t%d\n";
	en[CODESIGN_SLOTS] = "\tslots: \t\t%d\n";
	en[CODESIGN_SLOT_HEADER] = "\n  > %s: \n";
	en[CODESIGN_SLOT_TYPE] = "\ttype: \t\t0x%x\n";
	en[CODESIGN_SLOT_OFFSET] = "\toffset: \t%u\n";
	en[CODESIGN_SLOT_MAGIC] = "\tmagic: \t\t0x%x\n";
	en[CODESIGN_SLOT_LENGTH] = "\tlength: \t%u\n";
	en[CODESIGN_ENTITLEMENTS] = "\tentitlements: \n%s\n";
	en[CODESIGN_VERSION] = "\tversion: \t0x%x\n";
	en[CODESIGN_FLAGS] = "\tflags: \t\t%u\n";
	en[CODESIGN_HASH_OFFSET] = "\thashOffset: \t%u\n";
	en[CODESIGN_IDENT_OFFSET] = "\tidentOffset: \t%u\n";
	en[CODESIGN_N_SPECIAL_SLOTS] = "\tnSpecialSlots: \t%u\n";
	en[CODESIGN_N_CODE_SLOTS] = "\tnCodeSlots: \t%u\n";
	en[CODESIGN_CODE_LIMIT] = "\tcodeLimit: \t%u\n";
	en[CODESIGN_HASH_SIZE] = "\thashSize: \t%u\n";
	en[CODESIGN_HASH_TYPE] = "\thashType: \t%u\n";
	en[CODESIGN_SPARE1] = "\tspare1: \t%u\n";
	en[CODESIGN_PAGE_SIZE] = "\tpageSize: \t%u\n";
	en[CODESIGN_SPARE2] = "\tspare2: \t%u\n";
	en[CODESIGN_SCATTER_OFFSET] = "\tscatterOffset: \t%u\n";
	en[CODESIGN_TEAM_OFFSET] = "\tteamOffset: \t%u\n";
	en[CODESIGN_SPARE3] = "\tspare3: \t%u\n";
	en[CODESIGN_CODE_LIMIT64] = "\tcodeLimit64: \t%llu\n";
	en[CODESIGN_EXEC_SEG_BASE] = "\texecSegBase: \t%llu\n";
	en[CODESIGN_EXEC_SEG_LIMIT] = "\texecSegLimit: \t%llu\n";
	en[CODESIGN_EXEC_SEG_FLAGS] = "\texecSegFlags: \t%llu\n";
	en[CODESIGN_IDENTIFIER] = "\tidentifier: \t%s\n";
	en[CODESIGN_TEAMID] = "\tteamid: \t%s\n";
	en[CODESIGN_SPECIAL_SLOTS] = "\tSpecialSlots:\n";
	en[CODESIGN_CODE_SLOTS] = "\tCodeSlots:\n";
	en[CODESIGN_CODE_SLOTS_OMITTED] = "\tCodeSlots: \tomitted. (use -d option for details)\n";
	en[CODESIGN_CERTIFICATES] = "\tCertificates: \n";
	en[CODESIGN_SIGNED_ATTRS] = "\tSignedAttrs: \n";
	en[CODESIGN_CERT_ENTRY] = "\t\t\t%s\t<=\t%s\n";
	en[CODESIGN_ATTR_CONTENT_TYPE] = "\t  ContentType: \t%s => %s\n";
	en[CODESIGN_ATTR_SIGNING_TIME] = "\t  SigningTime: \t%s => %s\n";
	en[CODESIGN_ATTR_MSG_DIGEST] = "\t  MsgDigest: \t%s => %s\n";
	en[CODESIGN_ATTR_CDHASHES] = "\t  CDHashes: \t%s => \n\t\t\t\t%s\n";
	en[CODESIGN_ATTR_CDHASHES2] = "\t  CDHashes2: \t%s => \n";
	en[CODESIGN_ATTR_CDHASHES2_ITEM] = "\t\t\t\t%s\n";
	en[CODESIGN_ATTR_UNKNOWN] = "\t  UnknownAttr: \t%s => %s, type: %d, count: %d\n";
	en[CODESIGN_NEWLINE] = "\n";
	
	// archive.cpp
	en[ZIP_FAILED_OPEN_FILE] = "➤ Zip: Failed to open file: %s\n";
	en[ZIP_FAILED_ADD_FILE] = "➤ Zip: Failed to add file to zip: %s\n";
	en[ZIP_FAILED_CREATE_FOLDER] = "➤ Zip: Failed to create folder to zip: %s\n";
	en[ZIP_INVALID_COMPRESSION_LEVEL] = "➤ Zip: Invalid compression level: %d\n";
	en[ZIP_FAILED_CREATE] = "➤ Zip: Failed to create zip file: %s\n";
	
	// fs.cpp
	en[WRITEFILE_FOPEN_FAILED] = "WriteFile: Failed in fopen! %s, %s\n";
	en[APPENDFILE_FOPEN_FAILED] = "AppendFile: Failed in fopen! %s, %s\n";
	
	// util.cpp
	en[SYSTEM_EXEC_ERROR] = "SystemExec: \"%s\", error!\n";
	
	// metadata.cpp
	en[GETMETADATA_CANT_READ] = "➤ GetMetadata: Can't read %s\n";
	en[GETMETADATA_CANT_WRITE] = "➤ GetMetadata: Can't write %s\n";
	en[METADATA_PATH] = "➤ Metadata: %s\n";
	
	// certcheck.cpp
	en[CERT_SIGNED] = "➤ Signed: %s\n";
	en[CERT_NAME] = "➤ Name: %s\n";
	en[CERT_TYPE] = "➤ Type: %s\n";
	en[CERT_ORG] = "➤ Org: %s\n";
	en[CERT_TEAM] = "➤ Team: %s\n";
	en[CERT_SERIAL] = "➤ Serial: %s\n";
	en[CERT_ISSUED] = "➤ Issued: %s\n";
	en[CERT_EXPIRES_EXPIRED] = "➤ Expires: %s (EXPIRED %d days ago)\n";
	en[CERT_EXPIRES_REMAINING_WARN] = "➤ Expires: %s (%d days remaining!)\n";
	en[CERT_EXPIRES_REMAINING] = "➤ Expires: %s (%d days remaining)\n";
	en[CERT_ALGORITHM] = "➤ Algorithm: %s\n";
	en[CERT_ISSUER] = "➤ Issuer: %s\n";
	en[OCSP_VALID] = "➤ OCSP: Valid (ocsp.apple.com)\n";
	en[OCSP_REVOKED] = "➤ OCSP: REVOKED\n";
	en[OCSP_REVOKED_TIME] = "➤ Revoked: %s\n";
	en[OCSP_UNKNOWN] = "➤ OCSP: Unknown\n";
	en[OCSP_ERROR] = "➤ OCSP: Error\n";
	en[OCSP_DETAIL] = "➤ Detail: %s\n";
	en[CERT_CANT_READ_FILE] = "➤ Cannot read file: %s\n";
	en[CERT_UNKNOWN_FILE_TYPE] = "➤ Unknown file type: %s\n";
	en[CERT_CHECK] = "\n➤ Check: %s (%s)\n";
	en[CERT_SIGNED_NO] = "➤ Signed: No\n\n";
	en[CERT_LOAD_FAILED] = "➤ Failed to load certificate from %s\n";
	en[OCSP_SKIPPED] = "➤ OCSP: Skipped (non-WWDR issuer)\n";
	en[CERT_YES] = "Yes";
	en[CERT_NO] = "No";
	
	// timer.cpp 耗时格式
	en[TIME_FMT_S] = "%.3fs";
	en[TIME_FMT_M_S] = "%llum %.3fs";
	en[TIME_FMT_H_S] = "%lluh %.3fs";
	en[TIME_FMT_H_M_S] = "%lluh %llum %.3fs";
	en[TIME_FMT_WRAP] = "%s (%s, %lluus)\n";
}

const char* ZL10n::Get(const char* key) {
	return GetFmt(key);
}

const char* ZL10n::GetFmt(const char* key) {
	InitStrings();
	auto it = g_strings.find(g_locale);
	if (it == g_strings.end()) it = g_strings.find("en");
	if (it == g_strings.end()) return key;
	auto kit = it->second.find(key);
	if (kit == it->second.end()) return key;
	return kit->second.c_str();
}
