# ZsignIPAX

基于 [zsign](https://github.com/zhlynn/zsign) master（`614caa8`）的 Swift Package，为 iOS / macOS 应用提供 IPA 重签名与 Mach-O 工具能力。本分支在共享 `Core/` + `bridge/` 之上，额外提供面向 App 集成的高层 Swift API（`ZsignIPAX` 产品）。

> 本目录对应仓库 **`ipax-bridge` 分支**。若只需与 CLI 完全对齐的 `Zsign.run` 流程，请使用同仓库的 **`swift` 分支**（`ZsignSwift` 产品）。

## 功能特性

- **高层签名 API** — `sign` / `signIPA`，参数直观，适合 App 内调用
- **Mach-O 工具** — 注入 / 移除 / 列举 / 修改 dylib 路径
- **IPA 解压与打包** — `extractIPA` / `archiveFolderToIPA`
- **证书吊销检查** — `checkRevokage`（P12 + 描述文件 OCSP）
- **中文日志** — `zh: true` 或系统 `LANG=zh*` 自动中文化
- **压缩 / 解压进度** — 条目级进度与大文件 MB 心跳
- **用户取消** — `requestZipArchiveCancel()` 支持解压与打包阶段取消
- **签名前清理** — 内置 junk 清理（签 `.app` / IPA 流程内生效）
- **签后校验** — `sign` / `signIPA` 内部 `check=true`，校验 CodeDirectory 与 CMS 结构
- **最小 Core 改动** — 增强逻辑尽量放在 `bridge/`，便于同步上游

## 支持平台

| 平台 | 最低版本 |
|------|----------|
| iOS | 12.0 |
| macOS | 10.15 |
| tvOS | 12.0 |
| watchOS | 8.0 |

依赖：[krzyzanowskim/OpenSSL](https://github.com/krzyzanowskim/OpenSSL)（SPM 自动解析）。

## SPM 产品

| 产品 | 导入 | 说明 |
|------|------|------|
| **ZsignIPAX** | `import ZsignIPAX` | **推荐**：App 集成用高层 API |
| ZsignSwift | `import ZsignSwift` | CLI 对齐的 `Zsign.run` + `ZsignOptions` |
| Zsign | `import Zsign` | C/ObjC 桥接头（高级集成） |

## 集成

### Swift Package Manager

```swift
dependencies: [
    .package(url: "https://github.com/iStarCc/zsign.git", branch: "ipax-bridge"),
],
targets: [
    .target(
        name: "YourApp",
        dependencies: [
            .product(name: "ZsignIPAX", package: "Zsign"),
        ]
    ),
]
```

Xcode：**File → Add Package Dependencies** → `https://github.com/iStarCc/zsign.git`，选择 **`ipax-bridge`** 分支。

本地路径示例（与本仓库并列目录时）：

```swift
.package(path: "../Github/zsign/ipax-bridge"),
```

### 导入

```swift
import ZsignIPAX
```

## 快速开始

### 签名 IPA 并输出新包

```swift
import ZsignIPAX

let ok = Zsign.signIPA(
    inputPath: "/path/to/input.ipa",
    outputPath: "/path/to/output.ipa",
    provisionPath: "/path/to/app.mobileprovision",
    p12Path: "/path/to/cert.p12",
    p12Password: "p12密码",
    zipLevel: 6,
    zh: true,
    logHandler: { print($0, terminator: "") }
)

if ok {
    print("签名成功")
}
```

### 签名 .app 目录

```swift
let ok = Zsign.sign(
    appPath: "/path/to/MyApp.app",
    provisionPath: "/path/to/app.mobileprovision",
    p12Path: "/path/to/cert.p12",
    p12Password: "p12密码",
    customIdentifier: "com.example.app",
    zh: true
)
```

### 注入 dylib 后签名

```swift
_ = Zsign.injectDyLib(
    appExecutable: "/path/to/MyApp.app/MyApp",
    with: "@executable_path/Frameworks/libfoo.dylib"
)

_ = Zsign.sign(appPath: "/path/to/MyApp.app", /* ... */)
```

## 主要 API（ZsignIPAX）

| API | 说明 |
|-----|------|
| `Zsign.sign(...)` | 签名 `.app` 目录；内部 `check=true`、`force=true` |
| `Zsign.signIPA(...)` | 解压 → 签名 → 打包输出 IPA |
| `Zsign.extractIPA(...)` | 解压 `.ipa` 到目录 |
| `Zsign.archiveFolderToIPA(...)` | 将 `Payload` 目录打包为 IPA |
| `Zsign.checkSigned(appExecutable:)` | Mach-O 是否已签名 |
| `Zsign.injectDyLib` / `removeDylibs` / `listDylibs` / `changeDylibPath` | Mach-O dylib 操作 |
| `Zsign.checkRevokage(...)` | P12 + 描述文件 OCSP 吊销检查（异步） |
| `Zsign.requestZipArchiveCancel()` | 请求取消当前解压 / 打包 |
| `Zsign.zipArchiveLastFailureWasUserCancel()` | 上一轮失败是否为用户取消 |
| `Zsign.version` | 版本字符串 |
| `Zsign.setLogHandler(_:)` | 全局日志回调 |

`sign` / `signIPA` 支持 `zh:`、`logHandler`、`completion`；签后校验在内部始终启用（CodeDirectory + CMS 结构及证书检查）。

## 目录结构

```
ipax-bridge/
├── Core/                    # zsign C++ 核心（相对 master 改动见 CORE_MODIFICATIONS.md）
├── bridge/                  # overlay：日志 i18n、archive 进度/取消、IPA 清理、扩展 API
├── Sources/
│   ├── ZsignIPAX/           # 高层 Swift API（本分支主产品）
│   ├── ZsignIPAXBridgeCore/ # ObjC++ 桥接核心
│   ├── Zsign.swift          # ZsignSwift（CLI 对齐）
│   └── ZsignOptions.swift
├── Tests/ZsignIPAXTests/
├── docs/
└── Package.swift
```

## 本地构建与测试

```bash
cd ipax-bridge
swift build
swift test
```

> 集成测试（如 `ZsignIPAXTests`）需本地放置证书、IPA 等资源；缺少资源时相关用例会自动 `XCTSkip`。

## 分支对照

| 分支 | SPM 产品 | 适用场景 |
|------|----------|----------|
| **`ipax-bridge`** | **ZsignIPAX** | App 内签名、Mach-O 工具、解压/打包 |
| `swift` | ZsignSwift | 通用集成；CLI 对齐 `Zsign.run` |
| `master` | — | 原生 C++ CLI |

Core / bridge 有更新时，建议将 `swift` 分支的改动 merge 进 `ipax-bridge`，保持两套 API 与核心同步。

## 文档

| 文档 | 说明 |
|------|------|
| [docs/USAGE_GUIDE.md](docs/USAGE_GUIDE.md) | 完整 API、选项表与场景示例 |
| [CORE_MODIFICATIONS.md](CORE_MODIFICATIONS.md) | Core 相对 master 的修改与 bridge overlay 说明 |

## 上游与协议

- 上游项目：[zhlynn/zsign](https://github.com/zhlynn/zsign)
- 协议：MIT（与 zsign 一致）
