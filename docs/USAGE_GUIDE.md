# ZsignSwift 用法指南

Swift Package，底层为 master `zsign` C++ 核心，公开 API 与 CLI 选项对齐。

---

## 目录

1. [集成](#集成)
2. [快速开始](#快速开始)
3. [公开 API](#公开-api)
4. [ZsignOptions 选项参考](#zsignoptions-选项参考)
5. [输入类型与执行流程](#输入类型与执行流程)
6. [常见场景示例](#常见场景示例)
7. [日志与回调](#日志与回调)
8. [返回值与错误](#返回值与错误)
9. [平台说明](#平台说明)
10. [与 CLI 对照](#与-cli-对照)
11. [相关文档](#相关文档)

---

## 集成

### Swift Package Manager

在 Xcode：**File → Add Package Dependencies**，填入仓库地址（本地或远程）：

```
/path/to/Github/zsign/swift
```

或远程（推送后）：

```
https://github.com/iStarCc/zsign.git
```

分支/路径指向 `swift` 目录（若 monorepo 需指定 `swift` 子目录）。

`Package.swift` 依赖示例：

```swift
dependencies: [
    .package(path: "../Github/zsign/swift"),
],
targets: [
    .target(
        name: "YourApp",
        dependencies: [
            .product(name: "ZsignSwift", package: "Zsign"),
        ]
    ),
]
```

### 导入

```swift
import ZsignSwift
```

仅需 C/ObjC 桥接时（一般不直接用于 App）：

```swift
import Zsign
```

### 支持平台

| 平台 | 最低版本 |
|------|----------|
| iOS | 12.0 |
| macOS | 10.15 |
| tvOS | 12.0 |
| watchOS | 8.0 |

依赖：[krzyzanowskim/OpenSSL](https://github.com/krzyzanowskim/OpenSSL)（SPM 自动解析）。

---

## 快速开始

### 签名 IPA 并输出新包

```swift
import ZsignSwift

var options = ZsignOptions()
options.pkey = "/path/to/cert.p12"
options.password = "p12密码"
options.provisionPaths = ["/path/to/app.mobileprovision"]
options.output = "/path/to/output.ipa"
options.bundleId = "com.example.app"
options.zipLevel = 6

let code = Zsign.run(inputPath: "/path/to/input.ipa", options: options)
if code == 0 {
    print("签名成功")
} else {
    print("签名失败")
}
```

### 查看版本与帮助

```swift
print(Zsign.version)       // 例如 "version: 614caa8"
print(Zsign.helpText())      // 与 CLI -h 相同文本
```

---

## 公开 API

`ZsignSwift` 提供与 zsign CLI 对齐的公开 API：`Zsign.run` 及版本、帮助、日志、压缩/解压取消等。

### CLI 对齐 API

### `Zsign.version`

- **类型**：`String`
- **对应 CLI**：`-v` / `--version`
- **说明**：编译期嵌入的版本字符串

### `Zsign.helpText(zh:)`

- **类型**：`(zh: Bool = false) -> String`
- **对应 CLI**：`-h` / `--help`
- **说明**：返回帮助文本；`zh: true` 时临时设置 `ZSIGN_LANG=zh` 后返回中文；否则若系统 `LANG=zh*` 也会自动中文

### 中文日志

实现位于 `bridge/log_overlay.cpp` + `bridge/i18n/zlog_i18n.cpp`，**不修改** `Core/`。

| 行为 | 说明 |
|------|------|
| `>>>` → `➤ ` | 所有日志行统一箭头前缀（空行、分隔线、Tab 续行除外） |
| `run(zh: true)` | 本次调用临时 `ZSIGN_LANG=zh`，强制中文 |
| 系统 `LANG=zh*` | 未设 `zh:` 时也会自动中文 |
| `logHandler` / stdout | 均输出 **已 i18n 处理** 的 UTF-8 文本 |

```swift
Zsign.run(
    inputPath: ipaPath,
    options: options,
    zh: true,
    logHandler: { line in print(line, terminator: "") }
)

print(Zsign.helpText(zh: true))
```

### `Zsign.setLogHandler(_:)`

- **类型**：`((String) -> Void)? -> Void`
- **说明**：注册全局实时日志回调；传 `nil` 关闭
- **注意**：回调可能在后台线程触发，更新 UI 请切主线程
- **`run` 的 `logHandler` 参数**：仅在该次 `run` 期间生效，结束后自动 `setLogHandler(nil)`

### 压缩/解压进度与用户取消

实现位于 `bridge/archive_overlay.*` + `bridge/archive_zip_progress.*`，**不修改** `Core/common/archive.cpp`。

| API | 说明 |
|-----|------|
| `Zsign.requestZipCancel()` | 请求取消当前 IPA **压缩或解压** |
| `Zsign.zipLastFailureWasUserCancel()` | 上一轮压缩/解压失败是否因用户取消 |

**进度日志**（经 `logHandler` / stdout 输出）：

- 压缩：`Compressing files (3/120): Foo.app/Info.plist (1/2 MB) overall (42%)`  
  中文：`正在压缩（3/120）： Info.plist（1/2 MB）总计（42%）`
- 解压：`Unzipping files …` / `正在解压（…）`

**取消行为**：

- 在 **解压输入 IPA** 与 **打包输出 IPA** 阶段均可生效
- 取消后 `run` 返回 `-1`，`completion` 的 `error` 为 `NSURLErrorCancelled`
- 每次开始解压/压缩前桥接层自动 `ZipBeginZipOperation()`，清除上一轮取消状态

```swift
Task {
    async let signTask = Task.detached {
        Zsign.run(inputPath: ipa, options: options, logHandler: { print($0, terminator: "") })
    }
    try? await Task.sleep(nanoseconds: 2_000_000_000)
    Zsign.requestZipCancel()
    let code = await signTask.value
    if code != 0, Zsign.zipLastFailureWasUserCancel() {
        print("用户已取消打包")
    }
}
```

### `Zsign.run(inputPath:options:zh:logHandler:completion:)`

- **类型**：

```swift
@discardableResult
static func run(
    inputPath: String,
    options: ZsignOptions = ZsignOptions(),
    zh: Bool = false,
    logHandler: ((String) -> Void)? = nil,
    completion: ((Bool, Error?) -> Void)? = nil
) -> Int32
```

- **参数**：
  - `inputPath`：待处理路径（Mach-O 文件、`.ipa`、`.app` 目录等），对应 CLI  positional 参数
  - `options`：全部 CLI 选项，见下表
  - `zh`：为本次调用启用中文日志（非 CLI 选项）
  - `logHandler`：可选，本次调用期间的日志回调（内容为已 i18n 的 UTF-8）
  - `completion`：可选，结束时回调 `(success, error)`
- **返回值**：`0` 成功，`-1` 失败（与 CLI 退出码一致）
- **对应 CLI**：完整 `zsign [options] file|folder` 流程

---

## ZsignOptions 选项参考

所有字段默认空/false；空字符串等价于未传该 CLI 选项。

| Swift 属性 | CLI | 类型 | 说明 |
|------------|-----|------|------|
| `pkey` | `-k` / `--pkey` | `String` | 私钥或 `.p12` 路径（PEM/DER） |
| `provisionPaths` | `-m` / `--prov` | `[String]` | 描述文件路径；**可多个**（扩展/App 多 provision） |
| `cert` | `-c` / `--cert` | `String` | 单独证书文件（PEM/DER） |
| `adhoc` | `-a` / `--adhoc` | `Bool` | 仅 Ad-hoc 签名 |
| `debug` | `-d` / `--debug` | `Bool` | 调试模式，生成 `.zsign_debug` |
| `force` | `-f` / `--force` | `Bool` | 强制签名，忽略缓存 |
| `output` | `-o` / `--output` | `String` | 输出 `.ipa` 路径 |
| `password` | `-p` / `--password` | `String` | p12/私钥密码 |
| `bundleId` | `-b` / `--bundle_id` | `String` | 新 Bundle ID |
| `bundleName` | `-n` / `--bundle_name` | `String` | 新显示名称 |
| `bundleVersion` | `-r` / `--bundle_version` | `String` | 新 Bundle 版本 |
| `entitlements` | `-e` / `--entitlements` | `String` | 新 entitlements plist 路径 |
| `icon` | `-I` / `--icon` | `String` | 替换主图标（PNG） |
| `zipLevel` | `-z` / `--zip_level` | `Int` | 输出 IPA 压缩级别 **0–9** |
| `dylibs` | `-l` / `--dylib` | `[String]` | 注入 dylib 路径；**可多个** |
| `removeDylibs` | `-D` / `--rm_dylib` | `[String]` | 移除 dylib 名称；**可多个** |
| `weakInject` | `-w` / `--weak` | `Bool` | 以 `LC_LOAD_WEAK_DYLIB` 注入 |
| `install` | `-i` / `--install` | `Bool` | 签名后用 `ideviceinstaller` 安装（**仅 macOS**） |
| `tempFolder` | `-t` / `--temp_folder` | `String` | 临时目录；空则用系统临时目录 |
| `sha256Only` | `-2` / `--sha256_only` | `Bool` | SHA256-only CodeDirectory；**默认 `true`** |
| `legacySHA1` | `-L` / `--legacy_sha1` | `Bool` | 双 SHA1+SHA256（iOS ≤10）；设为 `true` 时覆盖 `sha256Only` |
| `check` | `-C` / `--check` | `Bool` | 证书有效性 / OCSP 检查 |
| `metadata` | `-x` / `--metadata` | `String` | 导出元数据与图标到目录 |
| `removeProvision` | `-R` / `--rm_provision` | `Bool` | 签名后移除 embedded.mobileprovision |
| `enableDocs` | `-S` / `--enable_docs` | `Bool` | 启用文档浏览与文件共享 |
| `minVersion` | `-M` / `--min_version` | `String` | 设置 `MinimumOSVersion` |
| `removeExtensions` | `-E` / `--rm_extensions` | `Bool` | 移除 PlugIns/Extensions |
| `removeWatch` | `-W` / `--rm_watch` | `Bool` | 移除 Watch App |
| `removeUISD` | `-U` / `--rm_uisd` | `Bool` | 移除 `UISupportedDevices` |
| `injectExtensions` | `-P` / `--inject_extensions` | `Bool` | 向扩展内也注入 `dylibs` |
| `removePaths` | — | `[String]` | 签名前额外清理（Swift-only，见下文） |
| `quiet` | `-q` / `--quiet` | `Bool` | 静默（关闭日志输出） |

### `removePaths`（签名前清理 IPA 中多余无效文件）

实现位于 `bridge/fs_ipa_junk.*`，**不修改** `Core/common/fs.*`。在 `SignFolder` 前自动执行，不在打包输出 IPA 前二次清理。

**日志**（`zh: true` 或系统 `LANG=zh*` 时为中文）：

- 开始：`正在清理 IPA 中多余无效文件...`
- 每项：`已删除：<相对路径>`
- 汇总：`已清理 N 项。`（仅当有删除项时）

**内置 junk**（始终清理）：`__MACOSX`、`.DS_Store`、`._*`、`.zsign_cache`、`.Spotlight-V100` 等。

**自定义项语义**：

| 模式 | 行为 |
|------|------|
| 不含 `*` | 仅删除 `.app` **根目录下一层**直接子项（文件或文件夹），如 `.esign` → 只删 `MyApp.app/.esign` |
| 含 `*` | 在 `.app` **整棵子树**内递归，对 **basename** 做 `fnmatch(3)` 匹配 |

示例：

```swift
var options = ZsignOptions()
options.removePaths = [".esign", "*.esign*", "*WatchPlaceholder*"]
// ... pkey、output 等
Zsign.run(inputPath: ipaPath, options: options)
```

### 选项组合注意

- **`legacySHA1 == true`**：底层 `bSHA256Only = false`，与 CLI `-L` 一致
- **输入为 `.ipa`**：内部自动 `force = true`、禁用签名缓存、解压到临时目录（与 CLI 相同）
- **`-C` 且无 `pkey`/`provisionPaths`**：仅执行 `CheckCertificate`，不签名
- **签名流程中 `-C`**：签名成功后先执行 `VerifySignedBundle`（CodeDirectory + CMS 结构校验），再执行 `CheckSignedBinary`（OCSP/证书检查）
- **`-x`**：在归档成功且指定 `output` 后提取元数据

---

## 输入类型与执行流程

```
inputPath
    │
    ├─ Mach-O 单文件
    │     ├─ 无签名参数 → PrintInfo()，返回 0
    │     └─ 有签名/注入参数 → 注入/移除 dylib → Sign → 返回
    │
    ├─ .ipa (zip)
    │     └─ 解压 → SignFolder → Archive → [GetMetadata] → [Install]
    │
    └─ 目录 (.app / Payload 等)
          └─ SignFolder → [Archive] → [GetMetadata] → [Install]
```

- **Mach-O**：可直接对二进制签名或注入 dylib
- **IPA**：必须提供 `output`（除非 `install == true` 且仅需临时 ipa 安装）
- **目录**：可直接签名；若需打包 ipa 需设置 `output`

---

## 常见场景示例

### 1. 仅查看 Mach-O 签名信息

```swift
let code = Zsign.run(inputPath: "/path/to/binary")
// 无 pkey/provision/dylib 等参数时，等价于 CLI 直接运行 zsign binary
```

### 2. 签名 .app 目录（不打包 IPA）

```swift
var options = ZsignOptions()
options.pkey = "/path/to/key.p12"
options.password = "secret"
options.provisionPaths = ["/path/to/profile.mobileprovision"]

Zsign.run(inputPath: "/path/to/App.app", options: options)
```

### 3. IPA 重签 + 改 Bundle ID + 换图标

```swift
var options = ZsignOptions()
options.pkey = "/path/to/key.p12"
options.password = "secret"
options.provisionPaths = ["/path/to/profile.mobileprovision"]
options.output = "/path/to/resigned.ipa"
options.bundleId = "com.new.id"
options.icon = "/path/to/icon.png"
options.zipLevel = 9

Zsign.run(inputPath: "/path/to/original.ipa", options: options)
```

### 4. 注入 dylib 并弱链接

```swift
var options = ZsignOptions()
options.pkey = "/path/to/key.p12"
options.password = "secret"
options.provisionPaths = ["/path/to/profile.mobileprovision"]
options.output = "/path/to/out.ipa"
options.dylibs = ["/path/to/inject.dylib"]
options.weakInject = true
options.injectExtensions = true  // -P：扩展内也注入

Zsign.run(inputPath: "/path/to/app.ipa", options: options)
```

### 5. 移除 dylib

```swift
var options = ZsignOptions()
options.removeDylibs = ["CydiaSubstrate", "@executable_path/libfoo.dylib"]
// ... 其余签名参数
```

名称不含 `/` 时，底层自动加前缀 `@executable_path/`。

### 6. 多描述文件（主 App + Extension）

```swift
var options = ZsignOptions()
options.provisionPaths = [
    "/path/to/app.mobileprovision",
    "/path/to/extension.mobileprovision",
]
// ... pkey、output 等
```

### 7. 仅检查证书 / OCSP（-C）

```swift
var options = ZsignOptions()
options.check = true
options.password = "p12密码"  // 检查 p12 时需要

// 检查 ipa / Mach-O / mobileprovision / p12 等
let code = Zsign.run(inputPath: "/path/to/file.p12", options: options)
// CheckCertificate 返回码：0 有效，1 吊销，2 过期，-2 未签名，-1 错误
```

### 7b. 签名并开启 check（-C）：签后 CMS 校验 + OCSP

```swift
var options = ZsignOptions()
options.check = true
options.pkey = "/path/to/key.p12"
options.password = "secret"
options.provisionPaths = ["/path/to/profile.mobileprovision"]
options.output = "/path/to/signed.ipa"

Zsign.run(inputPath: "/path/to/input.ipa", options: options, zh: true)
// 签名成功后日志示例（中文）：
// ➤ 校验所有 Mach-O 嵌入签名（共 N 个）...
// ➤ 主程序嵌入签名完整性校验通过：EasyTier
```

签后校验由 `bridge/verify_signed_bundle.*` 实现；Ad-hoc 签名时不强制 CMS blob，仍校验 CodeDirectory。

### 8. 签名后检查 + 导出元数据

```swift
var options = ZsignOptions()
options.check = true
options.pkey = "/path/to/key.p12"
options.password = "secret"
options.provisionPaths = ["/path/to/profile.mobileprovision"]
options.output = "/path/to/signed.ipa"
options.metadata = "/path/to/metadata_out"

Zsign.run(inputPath: "/path/to/input.ipa", options: options)
```

### 9. Ad-hoc 签名

```swift
var options = ZsignOptions()
options.adhoc = true
options.output = "/path/to/adhoc.ipa"

Zsign.run(inputPath: "/path/to/app.ipa", options: options)
```

### 10. 带实时日志（SwiftUI / UIKit）

```swift
Zsign.run(
    inputPath: ipaPath,
    options: options,
    logHandler: { line in
        DispatchQueue.main.async {
            self.logLines.append(line)
        }
    },
    completion: { success, error in
        DispatchQueue.main.async {
            if success {
                self.showSuccess()
            } else {
                self.showError(error?.localizedDescription ?? "失败")
            }
        }
    }
)
```

### 11. 全局日志 handler

```swift
Zsign.setLogHandler { line in
    print("[zsign] \(line)", terminator: "")
}

// 多次 run 共用同一 handler
Zsign.run(inputPath: path1, options: options1)
Zsign.run(inputPath: path2, options: options2)

Zsign.setLogHandler(nil)  // 关闭
```

### 12. CLI 等价写法对照

**CLI：**

```bash
zsign -k key.p12 -p pass -m app.mobileprovision \
  -o out.ipa -b com.example.app -z 6 \
  -l inject.dylib -w -f input.ipa
```

**Swift：**

```swift
var o = ZsignOptions()
o.pkey = "key.p12"
o.password = "pass"
o.provisionPaths = ["app.mobileprovision"]
o.output = "out.ipa"
o.bundleId = "com.example.app"
o.zipLevel = 6
o.dylibs = ["inject.dylib"]
o.weakInject = true
o.force = true
Zsign.run(inputPath: "input.ipa", options: o)
```

---

## 日志与回调

| 机制 | 作用域 | 说明 |
|------|--------|------|
| `setLogHandler` | 全局，直到置 `nil` | 所有后续 `ZLog` 输出 |
| `run(logHandler:)` | 单次 `run` | `defer` 自动清理 |
| `run(completion:)` | 单次 `run` | 结束时 `(Bool, Error?)` |
| `options.quiet` | 单次 `run` | 等价 `-q`，关闭底层日志级别 |
| `options.debug` | 单次 `run` | 等价 `-d`，详细日志 + `.zsign_debug` |

日志内容为 UTF-8 字符串，含 ANSI 颜色码（终端场景）；UI 展示时可按需剥离。

---

## 返回值与错误

### `run` 返回值

| 值 | 含义 |
|----|------|
| `0` | 成功 |
| `-1` | 失败（路径无效、解压失败、签名失败等） |

### `-C` 模式（`CheckCertificate`）

| 值 | 含义 |
|----|------|
| `0` | 有效 |
| `1` | 已吊销 |
| `2` | 已过期 |
| `-2` | 未签名 |
| `-1` | 错误 |

### `completion` 错误域

失败时 `NSError` domain 为 `"Zsign"`，`code` 为 `-1`，`localizedDescription` 为 `"Operation failed"`。详细原因请结合 `logHandler` 或控制台 `ZLog` 输出。

---

## 平台说明

| 能力 | iOS / tvOS / watchOS | macOS |
|------|----------------------|-------|
| 签名 / 解压 / 打包 | 支持 | 支持 |
| `-C` 证书检查（含 OCSP） | 支持（需网络） | 支持 |
| `-i` install | **不支持**（返回失败并打日志） | 支持（需已安装 `ideviceinstaller`） |
| `-d` `.zsign_debug` | 写入当前工作目录 | 同左 |

在 iOS App 内签名时，路径通常需使用 App Sandbox 内可写目录（Documents / tmp），证书与 ipa 需先复制到可访问路径。

---

## 与 CLI 对照

| 项目 | master CLI | ZsignSwift |
|------|------------|------------|
| 选项集 | 全部 long/short opt | `ZsignOptions` 全字段 |
| 主流程 | `main()` | `Zsign.run()` → `zsignRun()` |
| 版本 / 帮助 | `-v` / `-h` | `version` / `helpText()` |
| 默认 SHA256 | `-2` 默认开启 | `sha256Only = true` |
| 中文日志 | zlog_i18n | `run(zh:)` + `bridge/i18n` |
| IPA 多余文件清理 | — | `removePaths` + `bridge/fs_ipa_junk.*` |
| 签后 CMS 校验 | — | `check` + `bridge/verify_signed_bundle.*` |

---

## 相关文档

| 文档 | 说明 |
|------|------|
| [CORE_MODIFICATIONS.md](../CORE_MODIFICATIONS.md) | Core 相对 master 的修改；日志 i18n 在 bridge overlay |
| [master/src/zsign.cpp](../../master/src/zsign.cpp) | CLI 参考实现 |

---

## 命令行本地验证

```bash
cd Github/zsign/swift
swift build
swift test
```

---

**创建日期**: 2026-08-26  
**最后更新**: 2026-08-28  
**版本**: v0.5.2  
**状态**: 已完成
