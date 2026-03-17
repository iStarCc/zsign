//
//  log_keys.h
//  zsign
//
//  日志消息键，用于 L10n 国际化
//

#ifndef log_keys_h
#define log_keys_h

namespace ZL10nKeys {

// zsign.cpp - 签名流程 / ResolveExecutablePath
constexpr const char* FAILED_RESOLVE_EXEC_PATH = "failed_resolve_exec_path";
constexpr const char* INVALID_MACHO_PATH = "invalid_macho_path";
constexpr const char* MODIFIED_CONTENT_AT = "modified_content_at";
constexpr const char* DYLIBS_IN = "dylibs_in";
constexpr const char* MACHO_SIGNED = "macho_signed";
constexpr const char* MACHO_NOT_SIGNED = "macho_not_signed";
constexpr const char* DYLIB_INJECTED_OK = "dylib_injected_ok";
constexpr const char* DYLIB_INJECT_FAILED = "dylib_inject_failed";
constexpr const char* DYLIBS_UNINSTALLED_OK = "dylibs_uninstalled_ok";
constexpr const char* NO_DYLIBS_FOUND = "no_dylibs_found";
constexpr const char* DYLIB_PATH_CHANGED_OK = "dylib_path_changed_ok";
constexpr const char* DYLIB_PATH_CHANGE_FAILED = "dylib_path_change_failed";

constexpr const char* INVALID_PATH = "invalid_path";
constexpr const char* INVALID_INPUT_PATH = "invalid_input_path";
constexpr const char* INVALID_TEMP_FOLDER = "invalid_temp_folder";
constexpr const char* OUTPUT_PATH_REQUIRED = "output_path_required";
constexpr const char* SIGNING = "signing";
constexpr const char* SIGNED_FMT = "signed_fmt";
constexpr const char* SIGNED_OK = "signed_ok";
constexpr const char* SIGNED_FAILED = "signed_failed";
constexpr const char* DONE = "done";
constexpr const char* UNZIP = "unzip";
constexpr const char* UNZIP_FAILED = "unzip_failed";
constexpr const char* UNZIP_OK = "unzip_ok";
constexpr const char* ARCHIVING = "archiving";
constexpr const char* ARCHIVE_OK = "archive_ok";
constexpr const char* ARCHIVE_PROGRESS = "archive_progress";
constexpr const char* ARCHIVE_FAILED = "archive_failed";
constexpr const char* FAILED_CREATE_PAYLOAD = "failed_create_payload";
constexpr const char* FAILED_COPY_APP = "failed_copy_app";
constexpr const char* DEBUG_OPTION = "debug_option";
constexpr const char* DEBUG_ARGUMENT = "debug_argument";

// bundle.cpp - 签名详情
constexpr const char* REMOVED_EMBEDDED_PROVISION = "removed_embedded_provision";
constexpr const char* SIGN_FILE = "sign_file";
constexpr const char* SKIP_NON_MACHO = "skip_non_macho";
constexpr const char* CANT_GET_BUNDLE_INFO = "cant_get_bundle_info";
constexpr const char* CANT_PARSE_EXECUTABLE = "cant_parse_executable";
constexpr const char* CREATE_CODERESOURCES_FAILED = "create_coderesources_failed";
constexpr const char* CANT_GET_SHASUM = "cant_get_shasum";
constexpr const char* WRITING_CODERESOURCES_FAILED = "writing_coderesources_failed";
constexpr const char* CANT_WRITE_PROVISION = "cant_write_provision";
constexpr const char* CANT_FIND_PLUGIN_INFO = "cant_find_plugin_info";
constexpr const char* BUNDLE_ID = "bundle_id";
constexpr const char* BUNDLE_ID_VALUE = "bundle_id_value";
constexpr const char* BUNDLE_ID_PLUGIN = "bundle_id_plugin";
constexpr const char* BUNDLE_ID_WK = "bundle_id_wk";
constexpr const char* BUNDLE_ID_EXT = "bundle_id_ext";
constexpr const char* CANT_FIND_APP_INFO = "cant_find_app_info";
constexpr const char* BUNDLE_NAME = "bundle_name";
constexpr const char* BUNDLE_VERSION = "bundle_version";
constexpr const char* ENABLED_DOCUMENTS = "enabled_documents";
constexpr const char* MIN_OS_VERSION = "min_os_version";
constexpr const char* REMOVED = "removed";
constexpr const char* REMOVED_UI_DEVICES = "removed_ui_devices";
constexpr const char* CANT_FIND_APP_FOLDER = "cant_find_app_folder";
constexpr const char* CANT_GET_INFO_PLIST = "cant_get_info_plist";
constexpr const char* SIGN_FOLDER = "sign_folder";
constexpr const char* SIGNING_APP = "signing_app";
constexpr const char* APP_NAME = "app_name";
constexpr const char* VERSION = "version";
constexpr const char* TEAM_ID = "team_id";
constexpr const char* SUBJECT_CN = "subject_cn";
constexpr const char* READ_CACHE = "read_cache";
constexpr const char* CACHE_YES = "cache_yes";
constexpr const char* CACHE_NO = "cache_no";

// 通用
constexpr const char* ADHOC = "adhoc";

// openssl.cpp - 证书/配置
constexpr const char* NCONF_NEW_FAILED = "nconf_new_failed";
constexpr const char* NCONF_LOAD_BIO_FAILED = "nconf_load_bio_failed";
constexpr const char* NCONF_GET_STRING_FAILED = "nconf_get_string_failed";
constexpr const char* UNKNOWN_ISSUER_HASH = "unknown_issuer_hash";
constexpr const char* CANT_READ_ENTITLEMENTS_FILE = "cant_read_entitlements_file";
constexpr const char* CANT_FIND_PROVISION_FILE = "cant_find_provision_file";
constexpr const char* CANT_FIND_TEAM_ID = "cant_find_team_id";
constexpr const char* CANT_LOAD_P12_OR_KEY = "cant_load_p12_or_key";
constexpr const char* CANT_FIND_PAIRED_CERT_KEY = "cant_find_paired_cert_key";
constexpr const char* CANT_FIND_CERT_SUBJECT_CN = "cant_find_cert_subject_cn";

// macho.cpp - Mach-O
constexpr const char* INVALID_ARCH_IN_FAT = "invalid_arch_in_fat";
constexpr const char* INVALID_MACHO_FILE = "invalid_macho_file";
constexpr const char* INVALID_MACHO_MAGIC = "invalid_macho_magic";
constexpr const char* CODESIGN_MUNMAP_FAILED = "codesign_munmap_failed";
constexpr const char* REALLOC_CODESIGN_SPACE = "realloc_codesign_space";
constexpr const char* REALLOC_CODESIGN_SPACE_SUCCESS = "realloc_codesign_space_success";
constexpr const char* REALLOC_CODESIGN_SPACE_FAILED = "realloc_codesign_space_failed";
constexpr const char* FAILED = "failed";
constexpr const char* SUCCESS = "success";
constexpr const char* INJECT_DYLIB = "inject_dylib";
constexpr const char* INJECT_WEAK = "inject_weak";
constexpr const char* CHANGE_DYLIB_PATH = "change_dylib_path";
constexpr const char* FAILED_CHANGE_PATH_ARCH = "failed_change_path_arch";
constexpr const char* SUCCESS_CHANGED_DYLIB_PATHS = "success_changed_dylib_paths";
constexpr const char* FOUND_DYLIBS = "found_dylibs";

// archo.cpp - 签名/Mach-O 信息
constexpr const char* FILE_NOT_SIGNED = "file_not_signed";
constexpr const char* FILE_SIGNED = "file_signed";
constexpr const char* MACHO_INFO = "macho_info";
constexpr const char* EMBEDDED_INFO_PLIST = "embedded_info_plist";
constexpr const char* CANT_FIND_CODESIGN_SEGMENT = "cant_find_codesign_segment";
constexpr const char* BUILD_CODESIGN_FAILED = "build_codesign_failed";
constexpr const char* NO_ENOUGH_CODESIGN_SPACE = "no_enough_codesign_space";
constexpr const char* CANT_FIND_LOADCMD_SPACE_CODESIGN = "cant_find_loadcmd_space_codesign";
constexpr const char* CANT_FIND_LOADCMD_SPACE_DYLIB = "cant_find_loadcmd_space_dylib";
constexpr const char* INSUFFICIENT_SPACE_UPDATE_DYLIB = "insufficient_space_update_dylib";
constexpr const char* DYLIB_PATH_CHANGED = "dylib_path_changed";
constexpr const char* OLD_DYLIB_PATH_NOT_FOUND = "old_dylib_path_not_found";
constexpr const char* LOAD_TYPE_CHANGED = "load_type_changed";

// archive.cpp - Zip
constexpr const char* ZIP_FAILED_OPEN_FILE = "zip_failed_open_file";
constexpr const char* ZIP_FAILED_ADD_FILE = "zip_failed_add_file";
constexpr const char* ZIP_FAILED_CREATE_FOLDER = "zip_failed_create_folder";
constexpr const char* ZIP_INVALID_COMPRESSION_LEVEL = "zip_invalid_compression_level";
constexpr const char* ZIP_FAILED_CREATE = "zip_failed_create";

// fs.cpp - 文件 I/O
constexpr const char* WRITEFILE_FOPEN_FAILED = "writefile_fopen_failed";
constexpr const char* APPENDFILE_FOPEN_FAILED = "appendfile_fopen_failed";

// util.cpp - 系统命令
constexpr const char* SYSTEM_EXEC_ERROR = "system_exec_error";

// metadata.cpp
constexpr const char* GETMETADATA_CANT_READ = "getmetadata_cant_read";
constexpr const char* GETMETADATA_CANT_WRITE = "getmetadata_cant_write";
constexpr const char* METADATA_PATH = "metadata_path";

// certcheck.cpp - 证书检查
constexpr const char* CERT_SIGNED = "cert_signed";
constexpr const char* CERT_NAME = "cert_name";
constexpr const char* CERT_TYPE = "cert_type";
constexpr const char* CERT_ORG = "cert_org";
constexpr const char* CERT_TEAM = "cert_team";
constexpr const char* CERT_SERIAL = "cert_serial";
constexpr const char* CERT_ISSUED = "cert_issued";
constexpr const char* CERT_EXPIRES_EXPIRED = "cert_expires_expired";
constexpr const char* CERT_EXPIRES_REMAINING_WARN = "cert_expires_remaining_warn";
constexpr const char* CERT_EXPIRES_REMAINING = "cert_expires_remaining";
constexpr const char* CERT_ALGORITHM = "cert_algorithm";
constexpr const char* CERT_ISSUER = "cert_issuer";
constexpr const char* OCSP_VALID = "ocsp_valid";
constexpr const char* OCSP_REVOKED = "ocsp_revoked";
constexpr const char* OCSP_REVOKED_TIME = "ocsp_revoked_time";
constexpr const char* OCSP_UNKNOWN = "ocsp_unknown";
constexpr const char* OCSP_ERROR = "ocsp_error";
constexpr const char* OCSP_DETAIL = "ocsp_detail";
constexpr const char* CERT_CANT_READ_FILE = "cert_cant_read_file";
constexpr const char* CERT_UNKNOWN_FILE_TYPE = "cert_unknown_file_type";
constexpr const char* CERT_CHECK = "cert_check";
constexpr const char* CERT_SIGNED_NO = "cert_signed_no";
constexpr const char* CERT_LOAD_FAILED = "cert_load_failed";
constexpr const char* OCSP_SKIPPED = "ocsp_skipped";
constexpr const char* CERT_YES = "cert_yes";
constexpr const char* CERT_NO = "cert_no";

// timer.cpp - 耗时格式
constexpr const char* TIME_FMT_S = "time_fmt_s";
constexpr const char* TIME_FMT_M_S = "time_fmt_m_s";
constexpr const char* TIME_FMT_H_S = "time_fmt_h_s";
constexpr const char* TIME_FMT_H_M_S = "time_fmt_h_m_s";
constexpr const char* TIME_FMT_WRAP = "time_fmt_wrap";

// zsign.cpp - 额外
constexpr const char* INVALID_ZIP_LEVEL = "invalid_zip_level";
constexpr const char* CANT_FIND_PAYLOAD = "cant_find_payload";
constexpr const char* FAILED_INIT_PROVISION = "failed_init_provision";
constexpr const char* DYLIB_LIST_ITEM = "dylib_list_item";

// archo.cpp - Mach-O PrintInfo 结构标签
constexpr const char* MACHO_SEPARATOR = "macho_separator";
constexpr const char* MACHO_FILETYPE = "macho_filetype";
constexpr const char* MACHO_TOTALSIZE = "macho_totalsize";
constexpr const char* MACHO_PLATFORM = "macho_platform";
constexpr const char* MACHO_CPUARCH = "macho_cpuarch";
constexpr const char* MACHO_CPUTYPE = "macho_cputype";
constexpr const char* MACHO_CPUSUBTYPE = "macho_cpusubtype";
constexpr const char* MACHO_BIGENDIAN = "macho_bigendian";
constexpr const char* MACHO_ENCRYPTED = "macho_encrypted";
constexpr const char* MACHO_CMDCOUNT = "macho_cmdcount";
constexpr const char* MACHO_CODELENGTH = "macho_codelength";
constexpr const char* MACHO_SIGNLENGTH = "macho_signlength";
constexpr const char* MACHO_SPARELENGTH = "macho_sparelength";
constexpr const char* MACHO_MIN_IPHONEOS = "macho_min_iphoneos";
constexpr const char* MACHO_LC_RPATH = "macho_lc_rpath";
constexpr const char* MACHO_LC_LOAD_DYLIB = "macho_lc_load_dylib";
constexpr const char* MACHO_LC_LOAD_WEAK_DYLIB = "macho_lc_load_weak_dylib";
constexpr const char* MACHO_DYLIB_ITEM = "macho_dylib_item";
constexpr const char* MACHO_DYLIB_WEAK_ITEM = "macho_dylib_weak_item";
constexpr const char* MACHO_PLIST_LENGTH = "macho_plist_length";
constexpr const char* MACHO_PLIST_CONTENT = "macho_plist_content";
constexpr const char* MACHO_DYLIB_CLEAR = "macho_dylib_clear";

// signing.cpp - CodeSignature
constexpr const char* CODESIGN_SEGMENT_HEADER = "codesign_segment_header";
constexpr const char* CODESIGN_MAGIC = "codesign_magic";
constexpr const char* CODESIGN_LENGTH = "codesign_length";
constexpr const char* CODESIGN_SLOTS = "codesign_slots";
constexpr const char* CODESIGN_SLOT_HEADER = "codesign_slot_header";
constexpr const char* CODESIGN_SLOT_TYPE = "codesign_slot_type";
constexpr const char* CODESIGN_SLOT_OFFSET = "codesign_slot_offset";
constexpr const char* CODESIGN_SLOT_MAGIC = "codesign_slot_magic";
constexpr const char* CODESIGN_SLOT_LENGTH = "codesign_slot_length";
constexpr const char* CODESIGN_ENTITLEMENTS = "codesign_entitlements";
constexpr const char* CODESIGN_VERSION = "codesign_version";
constexpr const char* CODESIGN_FLAGS = "codesign_flags";
constexpr const char* CODESIGN_HASH_OFFSET = "codesign_hash_offset";
constexpr const char* CODESIGN_IDENT_OFFSET = "codesign_ident_offset";
constexpr const char* CODESIGN_N_SPECIAL_SLOTS = "codesign_n_special_slots";
constexpr const char* CODESIGN_N_CODE_SLOTS = "codesign_n_code_slots";
constexpr const char* CODESIGN_CODE_LIMIT = "codesign_code_limit";
constexpr const char* CODESIGN_HASH_SIZE = "codesign_hash_size";
constexpr const char* CODESIGN_HASH_TYPE = "codesign_hash_type";
constexpr const char* CODESIGN_SPARE1 = "codesign_spare1";
constexpr const char* CODESIGN_PAGE_SIZE = "codesign_page_size";
constexpr const char* CODESIGN_SPARE2 = "codesign_spare2";
constexpr const char* CODESIGN_SCATTER_OFFSET = "codesign_scatter_offset";
constexpr const char* CODESIGN_TEAM_OFFSET = "codesign_team_offset";
constexpr const char* CODESIGN_SPARE3 = "codesign_spare3";
constexpr const char* CODESIGN_CODE_LIMIT64 = "codesign_code_limit64";
constexpr const char* CODESIGN_EXEC_SEG_BASE = "codesign_exec_seg_base";
constexpr const char* CODESIGN_EXEC_SEG_LIMIT = "codesign_exec_seg_limit";
constexpr const char* CODESIGN_EXEC_SEG_FLAGS = "codesign_exec_seg_flags";
constexpr const char* CODESIGN_IDENTIFIER = "codesign_identifier";
constexpr const char* CODESIGN_TEAMID = "codesign_teamid";
constexpr const char* CODESIGN_SPECIAL_SLOTS = "codesign_special_slots";
constexpr const char* CODESIGN_CODE_SLOTS = "codesign_code_slots";
constexpr const char* CODESIGN_CODE_SLOTS_OMITTED = "codesign_code_slots_omitted";
constexpr const char* CODESIGN_CERTIFICATES = "codesign_certificates";
constexpr const char* CODESIGN_SIGNED_ATTRS = "codesign_signed_attrs";
constexpr const char* CODESIGN_CERT_ENTRY = "codesign_cert_entry";
constexpr const char* CODESIGN_ATTR_CONTENT_TYPE = "codesign_attr_content_type";
constexpr const char* CODESIGN_ATTR_SIGNING_TIME = "codesign_attr_signing_time";
constexpr const char* CODESIGN_ATTR_MSG_DIGEST = "codesign_attr_msg_digest";
constexpr const char* CODESIGN_ATTR_CDHASHES = "codesign_attr_cdhashes";
constexpr const char* CODESIGN_ATTR_CDHASHES2 = "codesign_attr_cdhashes2";
constexpr const char* CODESIGN_ATTR_CDHASHES2_ITEM = "codesign_attr_cdhashes2_item";
constexpr const char* CODESIGN_ATTR_UNKNOWN = "codesign_attr_unknown";
constexpr const char* CODESIGN_NEWLINE = "codesign_newline";

} // namespace ZL10nKeys

#endif /* log_keys_h */
