import Foundation

/// 与 master `zsign` CLI long option 一一对应的选项。
public struct ZsignOptions {
	public var pkey: String = ""
	public var provisionPaths: [String] = []
	public var cert: String = ""
	public var adhoc: Bool = false
	public var debug: Bool = false
	public var force: Bool = false
	public var output: String = ""
	public var password: String = ""
	public var bundleId: String = ""
	public var bundleName: String = ""
	public var bundleVersion: String = ""
	public var entitlements: String = ""
	public var icon: String = ""
	public var zipLevel: Int = 0
	public var dylibs: [String] = []
	public var removeDylibs: [String] = []
	/// 签名前额外清理项。无 `*`：仅 `.app` 根下直接子项；含 `*`：对 basename 在 `.app` 子树递归通配（如 `*.esign`、`*.esign*`）。
	public var removePaths: [String] = []
	public var weakInject: Bool = false
	public var install: Bool = false
	public var tempFolder: String = ""
	public var sha256Only: Bool = true
	public var legacySHA1: Bool = false
	public var check: Bool = false
	public var metadata: String = ""
	public var removeProvision: Bool = false
	public var enableDocs: Bool = false
	public var minVersion: String = ""
	public var removeExtensions: Bool = false
	public var removeWatch: Bool = false
	public var removeUISD: Bool = false
	public var injectExtensions: Bool = false
	public var quiet: Bool = false

	public init() {}
}
