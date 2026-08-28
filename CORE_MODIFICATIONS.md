# Core 相对 master 的修改

基线：`master/src` @ `614caa8`

Core 目录内 **仅保留** 以下与 master 不同的改动。日志 sink / 中文 i18n、压缩解压进度与用户取消 已移至 `bridge/`，**不再修改** `Core/common/log.*` 与 `Core/common/archive.*`（后者仅作上游参考，不参与编译）。

---

## 1. `common/mach-o.h` · 529–535 行

**master · 529–536 行**

```cpp
enum eSecRequirementType
{
	kSecHostRequirementType = 1,
	kSecGuestRequirementType = 2,
	kSecDesignatedRequirementType = 3,
	kSecLibraryRequirementType = 4,
};
```

**swift · 529–535 行**

```cpp
enum eZSignSecRequirementType
{
	kZSignSecHostRequirementType = 1,
	kZSignSecGuestRequirementType = 2,
	kZSignSecDesignatedRequirementType = 3,
	kZSignSecLibraryRequirementType = 4,
};
```

**原因**：`zsign.mm` 引入 `Foundation` → `Security/CSCommon.h`，与 master 枚举同名冲突，编译失败。

**影响**：常量数值不变（1–4）；仅符号重命名。

---

## 2. `signing.cpp` · 250 行

**master**

```cpp
appendBE32(kSecDesignatedRequirementType); // type = 3
```

**swift**

```cpp
appendBE32(kZSignSecDesignatedRequirementType); // type = 3
```

**原因**：配合 §1 枚举重命名。

**影响**：requirement blob type 仍为 3，与 master 二进制一致。

---

## bridge overlay（非 Core，不计入 Core 修改）

| 文件 | 说明 |
|------|------|
| [`bridge/log_overlay.cpp`](bridge/log_overlay.cpp) | 替代编译 `Core/common/log.cpp`；含 sink + `ZLogI18n::Apply` |
| [`bridge/log_overlay.h`](bridge/log_overlay.h) | `ZLog_SetExternalSink` 声明 |
| [`bridge/i18n/zlog_i18n.cpp`](bridge/i18n/zlog_i18n.cpp) | 中文词典与 `➤` 前缀（含 master 增量词条） |
| [`bridge/archive_overlay.cpp`](bridge/archive_overlay.cpp) | 替代编译 `Core/common/archive.cpp`；压缩/解压进度 + 用户取消 |
| [`bridge/archive_zip_progress.cpp`](bridge/archive_zip_progress.cpp) | `ZipLogCompressUnified` / `ZipLogExtractUnified` |
| [`bridge/archive_cancel.cpp`](bridge/archive_cancel.cpp) | `ZipBeginZipOperation` / `ZipRequestCancel` / `ZipLastFailureWasUserCancel` |
| [`bridge/fs_ipa_junk.cpp`](bridge/fs_ipa_junk.cpp) | 签名前清理 IPA 多余无效文件 + `ZsignOptions.removePaths` 自定义项 |
| [`bridge/verify_archo.cpp`](bridge/verify_archo.cpp) | CodeDirectory page hash 校验（基于 `ZArchO` public 字段） |
| [`bridge/verify_macho_map.cpp`](bridge/verify_macho_map.cpp) | 轻量 Mach-O 映射加载（不修改 Core `macho.cpp`） |
| [`bridge/verify_signed_bundle.cpp`](bridge/verify_signed_bundle.cpp) | 签后 CMS / CodeDirectory 完整性校验（`check == true` 时触发） |

### verify overlay 增强（均在 bridge，Core 不动）

- `VerifySignedBundle`：枚举 `.app` 内 Mach-O → 校验 code slots → 校验主程序 embedded signature（含 CMS blob 结构）
- 触发时机：**仅** `ZsignOptions.check == true`（CLI `-C`）
- Ad-hoc 签名：`bCheckCMS = false`，仍校验 CodeDirectory / code slots
- Swift 测试 API：`ZsignVerifySignedBundle(appFolder, bCheckCMS)`

### fs_ipa_junk overlay 增强（均在 bridge，Core 不动）

- 内置 junk：`__MACOSX`、`.DS_Store`、`._*`、`.zsign_cache` 等
- 自定义 `removePaths`：无 `*` 时仅删 `.app` 根下直接子项；含 `*` 时在 `.app` 子树对 basename 做 `fnmatch`
- 挂接点：`zsign.mm` 在 `SignFolder` 前调用，不在打包前二次清理
- Swift 测试 API：`ZsignRemoveIPAPackagingJunkFromFolder`

### archive overlay 相对 master `archive.cpp` 的增强（均在 bridge，Core 不动）

- 压缩/解压条目进度与大文件 MB 心跳
- `Zip::ExtractWithProgress` 替代 `zsign.mm` 中的 `Zip::Extract`
- 用户取消：`ZipRequestCancel()`，Swift 公开 API 为 `Zsign.requestZipCancel()`
- UTF-8 ZIP 条目名（`zipOpenNewFileInZip4_64` + bit 11）
- 保留 master 路径安全校验（含 `:` 拒绝）

---
**更新**: 2026-08-26
