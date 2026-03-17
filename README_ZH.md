# zsign

可能是 iOS 12+ 上最快的跨平台 codesign 替代方案，支持 macOS、Linux、Windows 及更多特性。[English](README.md)
如果觉得有帮助，请别忘了 <font color=#FF0000 size=5>🌟**star**🌟</font> [ME](https://github.com/zhlynn)。

## 编译

### macOS

```bash
brew install pkg-config openssl minizip
git clone https://github.com/zhlynn/zsign.git
cd zsign/build/macos
make clean && make
```

安装 `ideviceinstaller` 用于测试：
```bash
brew install ideviceinstaller
```

### Linux

#### Ubuntu 22.04 / Debian 12 / Mint 21

```bash
sudo apt-get install -y git g++ pkg-config libssl-dev libminizip-dev
git clone https://github.com/zhlynn/zsign.git
cd zsign/build/linux
make clean && make
```

安装 `ideviceinstaller` 用于测试：
```bash
sudo apt-get install -y ideviceinstaller
```

#### RHEL / CentOS / Alma / Rocky 等

需先安装 `epel-release`，例如：

RHEL / CentOS / Alma / Rocky 8：
```bash
sudo rpm -Uvh https://dl.fedoraproject.org/pub/epel/epel-release-latest-8.noarch.rpm
```

RHEL / CentOS / Alma / Rocky 9：
```bash
sudo rpm -Uvh https://dl.fedoraproject.org/pub/epel/epel-release-latest-9.noarch.rpm
```

然后安装依赖并编译：
```bash
sudo yum install -y git gcc-c++ pkg-config openssl-devel minizip1.2-devel
git clone https://github.com/zhlynn/zsign.git
cd zsign/build/linux
make clean && make
```

### Windows

使用 `Visual Studio 2022` 打开 `build/windows/vs2022/zsign.sln`，在 Windows 10/11 上编译。

## 使用方法

```bash
用法: zsign [-options] [-k privkey.pem] [-m dev.prov] [-o output.ipa] file|folder
选项:
-k, --pkey              私钥或 p12 文件路径（PEM 或 DER 格式）
-m, --prov              移动配置描述文件路径。可多次使用以支持 App 扩展
-c, --cert              证书文件路径（PEM 或 DER 格式）
-a, --adhoc             仅执行 ad-hoc 签名
-d, --debug             生成调试输出文件（.zsign_debug 目录）
-f, --force             签名目录时强制不使用缓存
-o, --output            输出 ipa 文件路径
-p, --password          私钥或 p12 文件密码
-b, --bundle_id         修改后的 Bundle ID
-n, --bundle_name       修改后的 Bundle 名称
-r, --bundle_version    修改后的 Bundle 版本
-e, --entitlements      修改后的 entitlements 文件
-z, --zip_level         输出 ipa 时的压缩级别（0-9）
-l, --dylib             注入 dylib。格式：-l "install_path" 或 -l "install_path=source_path"（= 及后可不写）。可多次使用
-D, --rm_dylib          要移除的 dylib 名称或路径。可多次使用。
                        - xxxx/xxx.dylib → 按 @executable_path/xxxx/xxx.dylib 匹配
                        - @executable_path/xxxx/xxx.dylib → 原样匹配
                        - @rpath/xxxx/xxx.dylib → 原样匹配
-w, --weak              以 LC_LOAD_WEAK_DYLIB 方式注入
-i, --install           使用 ideviceinstaller 安装 ipa 进行测试
-t, --temp_folder       临时文件目录

-2, --sha256_only       仅使用 SHA256 的单一 code directory
-C, --check             检查证书有效性及 OCSP 吊销状态
-x, --metadata          提取元数据和图标到指定目录
-R, --rm_provision      签名后移除 mobileprovision 文件
-S, --enable_docs       启用 UISupportsDocumentBrowser 和 UIFileSharingEnabled
-M, --min_version       设置 Info.plist 中的 MinimumOSVersion
-E, --rm_extensions     移除所有 App 扩展（PlugIns/Extensions）
-W, --rm_watch          移除 Watch 应用
-U, --rm_uisd           移除 Info.plist 中的 UISupportedDevices
-L, --list_dylibs       列出主可执行文件中的 dylib
-I, --inject_only       仅注入 dylib（不签名）。使用 -l。.ipa 需指定 -o
-J, --remove_only       仅移除 dylib（不签名）。使用 -D。.ipa 需指定 -o
    --no_remove_files   移除 dylib 时仅删除加载命令，不删除 dylib 文件
-y, --locale            语言：zh（简体中文）或 en（英文）。默认 en
-q, --quiet             静默模式
-v, --version           显示版本
-h, --help              显示帮助
```

## 示例

1. 显示 Mach-O 和 CodeSignature 段信息
```bash
./zsign demo.app/demo
```

2. 使用私钥和 mobileprovision 签名 ipa
```bash
./zsign -k privkey.pem -m dev.prov -o output.ipa -z 9 demo.ipa
```

3. 使用 p12 和 mobileprovision 签名目录（使用缓存）
```bash
./zsign -k dev.p12 -p 123 -m dev.prov -o output.ipa demo.app
```

4. 使用 p12 和 mobileprovision 签名目录（不使用缓存）
```bash
./zsign -f -k dev.p12 -p 123 -m dev.prov -o output.ipa demo.app
```

5. Ad-hoc 签名 ipa
```bash
./zsign -a -o output.ipa demo.ipa
```

6. 注入 dylib 后重新签名 ipa
```bash
./zsign -k dev.p12 -p 123 -m dev.prov -l "@executable_path/demo.dylib=./demo.dylib" -o output.ipa demo.ipa
```

7. 修改 Bundle ID 和 Bundle 名称
```bash
./zsign -k dev.p12 -p 123 -m dev.prov -b 'com.new.bundle.id' -n 'NewName' -o output.ipa demo.ipa
```

8. 签名 ipa 并提取元数据（应用信息 + 图标）到目录
```bash
./zsign -k dev.p12 -p 123 -m dev.prov -x ./metadata -o output.ipa demo.ipa
# 生成 ./metadata/metadata.json 和 ./metadata/<hash>.png
```

9. 启用文档支持（Files 应用集成）
```bash
./zsign -k dev.p12 -p 123 -m dev.prov -S -o output.ipa demo.ipa
```

10. 设置最低系统版本
```bash
./zsign -k dev.p12 -p 123 -m dev.prov -M 14.0 -o output.ipa demo.ipa
```

11. 移除所有 App 扩展（PlugIns/Extensions）
```bash
./zsign -k dev.p12 -p 123 -m dev.prov -E -o output.ipa demo.ipa
```

12. 移除 Watch 应用
```bash
./zsign -k dev.p12 -p 123 -m dev.prov -W -o output.ipa demo.ipa
```

13. 移除 UISupportedDevices 以允许任意设备安装
```bash
./zsign -k dev.p12 -p 123 -m dev.prov -U -o output.ipa demo.ipa
```

14. 列出主可执行文件中的 dylib（支持 Mach-O、.app、.ipa）
```bash
./zsign -L demo.app/demo
./zsign -L demo.app
./zsign -L demo.ipa
```

15. 仅注入 dylib，不签名
```bash
# -l "install_path" 或 "install_path=source_path"，= 及后可不写（不写则仅添加 load command 不复制文件）
# 支持 demo.app 或 demo.ipa；-o 可选（.ipa 输入时必填）
# 默认：LC_LOAD_WEAK_DYLIB；-a（adhoc）：LC_LOAD_DYLIB；-w：LC_LOAD_WEAK_DYLIB（显式）
./zsign -I -l "@rpath/MyLib.dylib=/path/to/MyLib.dylib" demo.app|demo.ipa [-o output.ipa]
./zsign -I -a -l "@rpath/MyLib.dylib=/path/to/MyLib.dylib" demo.app|demo.ipa [-o output.ipa]
./zsign -I -w -l "@rpath/MyLib.dylib=/path/to/MyLib.dylib" demo.app|demo.ipa [-o output.ipa]
```

16. 仅移除 dylib，不签名
```bash
# -D 格式：xxxx/xxx.dylib → @executable_path/xxxx/xxx.dylib；@executable_path/... 或 @rpath/... → 原样
# 文件删除：@executable_path/xxx → app/xxx；@rpath/xxx → app/Frameworks/xxx
# 支持 demo.app 或 demo.ipa；-o 可选（.ipa 输入时必填）
./zsign -J -D OldLib.dylib demo.app|demo.ipa [-o output.ipa]
./zsign -J -D "@rpath/OldLib.dylib" demo.app [-o output.ipa]
# 仅移除加载命令，不删除 dylib 文件：
./zsign -J -D "@rpath/OldLib.dylib" --no_remove_files demo.app [-o output.ipa]
```

## 证书检查 (-C)

`-C` 可检查任意支持文件的签名证书，并对 Apple 服务器执行 OCSP 吊销检查。可直接从 IPA 内读取二进制，无需解压。使用 `-y zh` 可输出中文。

**支持的文件类型：** `.ipa`、`.mobileprovision`、`.p12`/`.pfx`、`.cer`/`.pem`、Mach-O 二进制

17. 检查 IPA 文件
```bash
./zsign -C demo.ipa
```

18. 检查 mobileprovision 文件
```bash
./zsign -C dev.mobileprovision
```

19. 检查 P12/PFX 证书文件
```bash
./zsign -C dev.p12 -p 123
```

20. 直接检查 Mach-O 二进制
```bash
./zsign -C demo.app/demo
```

21. 签名 IPA 并在打包前验证证书
```bash
./zsign -C -k dev.p12 -p 123 -m dev.prov -o output.ipa demo.ipa
```

**输出示例：**
```
➤ 检查:      demo.ipa (IPA)
➤ 已签名:    是
➤ 名称:       Apple Distribution: Company Name (TEAMID)
➤ 类型:       Apple Distribution
➤ 组织:       Company Name
➤ 团队:       TEAMID
➤ 序列号:     XX:XX:XX:XX:XX:XX:XX:XX
➤ 签发时间:   2025-01-01T00:00:00Z
➤ 过期时间:   2026-01-01T00:00:00Z (365 天剩余)
➤ 算法:       RSA 2048-bit
➤ 签发者:     Apple Worldwide Developer Relations Certification Authority
➤ OCSP:       有效 (ocsp.apple.com)
```

## 如何快速签名？

先解压 IPA，再用 zsign 对包含资源的目录签名。首次签名时，zsign 会执行完整签名并将签名信息缓存到当前目录的 `.zsign_cache`。后续对同一目录重新签名时，会利用缓存显著提速，签名会非常快！不妨试试！

## 许可证

zsign 采用 MIT 许可证。详见 [LICENSE](LICENSE) 文件。
