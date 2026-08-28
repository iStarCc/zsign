import Foundation
import Zsign

public enum Zsign {
	public static var version: String {
		String(cString: ZsignVersionString())
	}

	public static func helpText(zh: Bool = false) -> String {
		if zh {
			return withTemporaryZSIGNLang("zh") {
				String(cString: ZsignHelpText())
			}
		}
		return String(cString: ZsignHelpText())
	}

	public static func setLogHandler(_ handler: ((String) -> Void)?) {
		if let handler {
			ZsignSetLogHandler { line in
				guard let line else { return }
				handler(line)
			}
		} else {
			ZsignSetLogHandler(nil)
		}
	}

	/// 请求取消当前正在进行的 IPA 压缩或解压。
	public static func requestZipCancel() {
		ZsignRequestZipCancel()
	}

	/// 上一轮压缩/解压失败是否因用户调用 `requestZipCancel()`。
	public static func zipLastFailureWasUserCancel() -> Bool {
		ZsignZipLastFailureWasUserCancel()
	}

	/// 完整复刻 master CLI；返回 0 成功，-1 失败。
	@discardableResult
	public static func run(
		inputPath: String,
		options: ZsignOptions = ZsignOptions(),
		zh: Bool = false,
		logHandler: ((String) -> Void)? = nil,
		completion: ((Bool, Error?) -> Void)? = nil
	) -> Int32 {
		if let logHandler {
			setLogHandler(logHandler)
		}
		defer {
			if logHandler != nil {
				setLogHandler(nil)
			}
		}

		let storage = ZsignCStringStorage(
			provisionPaths: options.provisionPaths,
			dylibs: options.dylibs,
			removeDylibs: options.removeDylibs,
			removePaths: options.removePaths
		)
		return storage.withOptions(options, zh: zh) { cOptions in
			Int32(zsignRun(inputPath, cOptions) { success, error in
				completion?(success, error)
			})
		}
	}
}

private func withTemporaryZSIGNLang<T>(_ value: String, _ body: () -> T) -> T {
	let key = "ZSIGN_LANG"
	let previous = getenv(key).map { String(cString: $0) }
	setenv(key, value, 1)
	defer {
		if let previous {
			setenv(key, previous, 1)
		} else {
			unsetenv(key)
		}
	}
	return body()
}

private extension String {
	var nullableCString: UnsafePointer<CChar>? {
		isEmpty ? nil : (self as NSString).utf8String
	}
}

private final class ZsignCStringStorage {
	private var provisionCopies: [UnsafeMutablePointer<CChar>?] = []
	private var dylibCopies: [UnsafeMutablePointer<CChar>?] = []
	private var removeCopies: [UnsafeMutablePointer<CChar>?] = []
	private var removePathCopies: [UnsafeMutablePointer<CChar>?] = []
	private var provisionPointers: [UnsafePointer<CChar>?] = []
	private var dylibPointers: [UnsafePointer<CChar>?] = []
	private var removePointers: [UnsafePointer<CChar>?] = []
	private var removePathPointers: [UnsafePointer<CChar>?] = []

	init(provisionPaths: [String], dylibs: [String], removeDylibs: [String], removePaths: [String]) {
		provisionCopies = provisionPaths.map { strdup($0) }
		dylibCopies = dylibs.map { strdup($0) }
		removeCopies = removeDylibs.map { strdup($0) }
		removePathCopies = removePaths.map { strdup($0) }
		provisionPointers = provisionCopies.map { $0.map { UnsafePointer($0) } }
		dylibPointers = dylibCopies.map { $0.map { UnsafePointer($0) } }
		removePointers = removeCopies.map { $0.map { UnsafePointer($0) } }
		removePathPointers = removePathCopies.map { $0.map { UnsafePointer($0) } }
	}

	deinit {
		for ptr in provisionCopies + dylibCopies + removeCopies + removePathCopies {
			if let ptr {
				free(ptr)
			}
		}
	}

	func withOptions<T>(_ options: ZsignOptions, zh: Bool, _ body: (UnsafePointer<ZsignRunOptions>) -> T) -> T {
		provisionPointers.withUnsafeBufferPointer { provBuffer in
			dylibPointers.withUnsafeBufferPointer { dylibBuffer in
				removePointers.withUnsafeBufferPointer { rmBuffer in
					removePathPointers.withUnsafeBufferPointer { removePathBuffer in
						var cOptions = makeOptions(
							options,
							zh: zh,
							provisionPaths: provBuffer.baseAddress,
							provisionPathCount: provBuffer.count,
							dylibs: dylibBuffer.baseAddress,
							dylibCount: dylibBuffer.count,
							removeDylibs: rmBuffer.baseAddress,
							removeDylibCount: rmBuffer.count,
							removePaths: removePathBuffer.baseAddress,
							removePathCount: removePathBuffer.count
						)
						return withUnsafePointer(to: &cOptions) { body($0) }
					}
				}
			}
		}
	}

	private func makeOptions(
		_ options: ZsignOptions,
		zh: Bool,
		provisionPaths: UnsafePointer<UnsafePointer<CChar>?>?,
		provisionPathCount: Int,
		dylibs: UnsafePointer<UnsafePointer<CChar>?>?,
		dylibCount: Int,
		removeDylibs: UnsafePointer<UnsafePointer<CChar>?>?,
		removeDylibCount: Int,
		removePaths: UnsafePointer<UnsafePointer<CChar>?>?,
		removePathCount: Int
	) -> ZsignRunOptions {
		ZsignRunOptions(
			pkey: options.pkey.nullableCString,
			cert: options.cert.nullableCString,
			password: options.password.nullableCString,
			bundleId: options.bundleId.nullableCString,
			bundleName: options.bundleName.nullableCString,
			bundleVersion: options.bundleVersion.nullableCString,
			entitlements: options.entitlements.nullableCString,
			icon: options.icon.nullableCString,
			output: options.output.nullableCString,
			tempFolder: options.tempFolder.nullableCString,
			metadata: options.metadata.nullableCString,
			minVersion: options.minVersion.nullableCString,
			provisionPaths: provisionPaths,
			provisionPathCount: provisionPathCount,
			dylibs: dylibs,
			dylibCount: dylibCount,
			removeDylibs: removeDylibs,
			removeDylibCount: removeDylibCount,
			removePaths: removePaths,
			removePathCount: removePathCount,
			zipLevel: Int32(options.zipLevel),
			adhoc: options.adhoc,
			debug: options.debug,
			force: options.force,
			weakInject: options.weakInject,
			install: options.install,
			sha256Only: options.sha256Only,
			legacySHA1: options.legacySHA1,
			check: options.check,
			removeProvision: options.removeProvision,
			enableDocs: options.enableDocs,
			removeExtensions: options.removeExtensions,
			removeWatch: options.removeWatch,
			removeUISD: options.removeUISD,
			injectExtensions: options.injectExtensions,
			quiet: options.quiet,
			zh: zh
		)
	}
}
