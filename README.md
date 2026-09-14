# zsign master-ipax

Backend 专用分支：在 **master `src/` 布局** 之上，通过 **`bridge/` 外层套接** 提供与 swift 版一致的日志、压缩进度、中文 i18n 与 `zsignRun` 引擎；CLI 选项与 upstream master 完全对齐。

> **不修改** 仓库内 `master` / `swift` / `ipax-bridge` 分支。iOS App 仍使用 `ipax-bridge` SPM。

## 构建

### macOS

```bash
cd build/macos
make clean && make
# 产物: ../../bin/zsign
```

### Linux

```bash
cd build/linux
make clean && make
```

### Windows

使用 `build/windows/vs2022/`（需将工程源列表改为：排除 `src/zsign.cpp`、`src/common/log.cpp`、`src/common/archive.cpp`，加入 `bridge/` 源与 `bridge/cli_main.cpp`）。

## 使用

与 master CLI 相同；设置 `ZSIGN_LANG=zh` 启用中文日志（Backend 已默认设置）。

```bash
export ZSIGN_LANG=zh
./bin/zsign -h
./bin/zsign -f -k cert.p12 -p password -m app.mobileprovision -o out.ipa -z 6 Payload/MyApp.app
```

## 目录

| 路径 | 说明 |
|------|------|
| `src/` | 与 master 对齐的上游 Core（不编译 `zsign.cpp` / `log.cpp` / `archive.cpp`） |
| `bridge/` | IPAX overlay：日志 i18n、archive 进度/取消、junk 清理、签后校验 |
| `bridge/cli_main.cpp` | CLI 入口 → `zsignRun()` |
| `bridge/zsign_run.cpp` | 纯 C++ 版 RunEngine（由 swift `zsign.mm` 移植） |

详见 [CORE_MODIFICATIONS.md](CORE_MODIFICATIONS.md)。

## Backend 集成

zsign **不在 Docker 镜像内编译**。本地或 CI 在构建镜像前单独执行：

```bash
./scripts/build-zsign-binaries.sh   # 或 Backend/scripts/build_zsign.sh
```

产物写入 `Backend/storage/cmd/{Linux,MacOS}/zsign`，随 `COPY Backend/` 打入镜像。二进制在 `.gitignore` 中，发布 CI 会在 `docker build` 前自动编译 Linux 版。

---
**分支**: master-ipax  
**基线**: master @ 614caa8
