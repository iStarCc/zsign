# ZsignSwift

基于 [zsign](https://github.com/zhlynn/zsign) master（`614caa8`）的 Swift Package，为 iOS / macOS 应用提供 IPA 重签名能力。公开 API 与 zsign CLI 选项对齐，并通过 `bridge/` overlay 扩展中文日志、压缩/解压进度、用户取消与签名前文件清理等能力。

> 本目录为独立的 **`swift` 分支** Swift Package，与 `master` 分支的 CLI 源码并列维护。

## 功能特性

- **完整 CLI 对齐** — `ZsignOptions` 覆盖 zsign 全部命令行选项
- **中文日志** — `run(zh: true)` 或系统 `LANG=zh*` 自动中文化
- **压缩 / 解压进度** — 条目级进度与大文件 MB 心跳
- **用户取消** — `Zsign.requestZipCancel()` 支持解压与打包阶段取消
- **签名前清理** — 内置 junk 清理 + `removePaths` 自定义规则（Swift-only）
- **签后 CMS 校验** — `check == true`（`-C`）时校验 CodeDirectory 与 CMS 结构
- **最小 Core 改动** — 增强逻辑尽量放在 `bridge/`，便于同步上游

## 支持平台

| 平台 | 最低版本 |
|------|----------|
| iOS | 12.0 |
| macOS | 10.15 |
| tvOS | 12.0 |
| watchOS | 8.0 |

依赖：[krzyzanowskim/OpenSSL](https://github.com/krzyzanowskim/OpenSSL)（SPM 自动解析）。

## 集成

### Swift Package Manager

```swift
dependencies: [
    .package(url: "https://github.com/iStarCc/zsign.git", branch: "swift"),
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

Xcode：**File → Add Package Dependencies** → `https://github.com/iStarCc/zsign.git`，选择 **`swift`** 分支。

### 导入

```swift
import ZsignSwift
```

## 快速开始

```swift
import ZsignSwift

var options = ZsignOptions()
options.pkey = "/path/to/cert.p12"
options.password = "p12密码"
options.provisionPaths = ["/path/to/app.mobileprovision"]
options.output = "/path/to/output.ipa"
options.removePaths = [".esign", "*.junk*"]  // 可选：签名前额外清理

let code = Zsign.run(
    inputPath: "/path/to/input.ipa",
    options: options,
    zh: true,
    logHandler: { print($0, terminator: "") }
)

if code == 0 {
    print("签名成功")
}
```

## 主要 API

| API | 说明 |
|-----|------|
| `Zsign.run(inputPath:options:zh:logHandler:completion:)` | 主流程，等价 CLI `zsign [options] file\|folder` |
| `Zsign.version` | 版本字符串 |
| `Zsign.helpText(zh:)` | 帮助文本 |
| `Zsign.setLogHandler(_:)` | 全局日志回调 |
| `Zsign.requestZipCancel()` | 请求取消当前解压 / 打包 |
| `Zsign.zipLastFailureWasUserCancel()` | 上一轮失败是否为用户取消 |
| `ZsignVerifySignedBundle(_:bCheckCMS:)` | 对已签 `.app` 做 CodeDirectory + CMS 校验（C API，测试/高级用法） |

`removePaths` 规则：

- **不含 `*`**：仅删除 `.app` 根目录下一层直接子项
- **含 `*`**：在 `.app` 子树内对 basename 做通配匹配

清理日志示例（中文）：

```
➤ 正在清理 IPA 中多余无效文件...
➤ 已删除：Payload/MyApp.app/.esign
➤ 已清理 1 项。
```

## 目录结构

```
swift/
├── Core/           # zsign C++ 核心（相对 master 改动见 CORE_MODIFICATIONS.md）
├── bridge/         # overlay：日志 i18n、archive 进度/取消、IPA 清理
├── Sources/        # ZsignSwift 公开 Swift API
├── docs/           # 详细文档
└── Package.swift
```

## 本地构建与测试

```bash
cd swift
swift build
swift test
```

> 仓库默认不包含 `Tests/` 目录；本地测试资源（证书、IPA 等）需自行放置。

## 文档

| 文档 | 说明 |
|------|------|
| [docs/USAGE_GUIDE.md](docs/USAGE_GUIDE.md) | 完整 API、选项表与场景示例 |
| [CORE_MODIFICATIONS.md](CORE_MODIFICATIONS.md) | Core 相对 master 的修改与 bridge overlay 说明 |

## 上游与协议

- 上游项目：[zhlynn/zsign](https://github.com/zhlynn/zsign)
- 协议：MIT（与 zsign 一致）
