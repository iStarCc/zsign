import Foundation
import Zsign
import ZsignIPAXBridgeCore

public enum Zsign {
	public static var version: String {
		String(cString: ZsignVersionString())
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

	public static func requestZipArchiveCancel() {
		ZsignRequestZipArchiveCancel()
	}

	public static func zipArchiveLastFailureWasUserCancel() -> Bool {
		ZsignZipArchiveLastFailureWasUserCancel()
	}

	public static func checkSigned(appExecutable: String) -> Bool {
		CheckIfSigned(appExecutable)
	}

	public static func injectDyLib(appExecutable: String, with path: String, weak: Bool = true) -> Bool {
		InjectDyLib(appExecutable, path, weak)
	}

	public static func removeDylibs(appExecutable: String, using dylibs: [String]) -> Bool {
		UninstallDylibs(appExecutable, dylibs)
	}

	public static func listDylibs(appExecutable: String) -> [String] {
		ListDylibs(appExecutable) ?? []
	}

	public static func changeDylibPath(appExecutable: String, for old: String, with new: String) -> Bool {
		ChangeDylibPath(appExecutable, old, new)
	}

	public static func sign(
		appPath: String = "",
		provisionPath: String = "",
		p12Path: String = "",
		p12Password: String = "",
		entitlementsPath: String = "",
		customIdentifier: String = "",
		customName: String = "",
		customVersion: String = "",
		adhoc: Bool = false,
		removeProvision: Bool = false,
		removeUISupportedDevices: Bool = false,
		removeWatchApp: Bool = false,
		enableDocuments: Bool = false,
		minOSVersion: String = "",
		removeExtensions: Bool = false,
		zh: Bool = false,
		logHandler: ((String) -> Void)? = nil,
		completion: ((Bool, Error?) -> Void)? = nil
	) -> Bool {
		withOptionalLogHandler(logHandler) {
			zsign(
				appPath,
				provisionPath,
				p12Path,
				p12Password,
				entitlementsPath,
				customIdentifier,
				customName,
				customVersion,
				adhoc,
				removeProvision,
				removeUISupportedDevices,
				removeWatchApp,
				enableDocuments,
				minOSVersion,
				removeExtensions,
				zh,
				completion.map { callback in
					{ success, error in callback(success, error) }
				}
			) == 0
		}
	}

	public static func signIPA(
		inputPath: String,
		outputPath: String,
		provisionPath: String = "",
		p12Path: String = "",
		p12Password: String = "",
		entitlementsPath: String = "",
		customIdentifier: String = "",
		customName: String = "",
		customVersion: String = "",
		adhoc: Bool = false,
		removeProvision: Bool = false,
		removeUISupportedDevices: Bool = false,
		removeWatchApp: Bool = false,
		enableDocuments: Bool = false,
		minOSVersion: String = "",
		removeExtensions: Bool = false,
		zipLevel: Int = 6,
		tempFolderPath: String = "",
		zh: Bool = false,
		logHandler: ((String) -> Void)? = nil,
		completion: ((Bool, Error?) -> Void)? = nil
	) -> Bool {
		withOptionalLogHandler(logHandler) {
			let zl = min(max(zipLevel, 0), 9)
			return zsignIPA(
				inputPath,
				outputPath,
				provisionPath,
				p12Path,
				p12Password,
				entitlementsPath,
				customIdentifier,
				customName,
				customVersion,
				adhoc,
				removeProvision,
				removeUISupportedDevices,
				removeWatchApp,
				enableDocuments,
				minOSVersion,
				removeExtensions,
				Int32(zl),
				tempFolderPath.isEmpty ? nil : tempFolderPath,
				zh,
				completion.map { callback in
					{ success, error in callback(success, error) }
				}
			) == 0
		}
	}

	public static func archiveFolderToIPA(
		folderPath: String,
		outputPath: String,
		zipLevel: Int = 6,
		zh: Bool = false,
		logHandler: ((String) -> Void)? = nil,
		completion: ((Bool, Error?) -> Void)? = nil
	) -> Bool {
		withOptionalLogHandler(logHandler) {
			let zl = min(max(zipLevel, 0), 9)
			return zsignArchiveFolderToIPA(
				folderPath,
				outputPath,
				Int32(zl),
				zh,
				completion.map { callback in
					{ success, error in callback(success, error) }
				}
			) == 0
		}
	}

	public static func extractIPA(
		ipaPath: String,
		outputFolderPath: String,
		zh: Bool = false,
		logHandler: ((String) -> Void)? = nil,
		completion: ((Bool, Error?) -> Void)? = nil
	) -> Bool {
		withOptionalLogHandler(logHandler) {
			zsignExtractIPA(
				ipaPath,
				outputFolderPath,
				zh,
				completion.map { callback in
					{ success, error in callback(success, error) }
				}
			) == 0
		}
	}

	public static func checkRevokage(
		provisionPath: String = "",
		p12Path: String = "",
		p12Password: String = "",
		completionHandler: @escaping (Int32, Date?, String?) -> Void
	) {
		checkCert(
			provisionPath,
			p12Path,
			p12Password
		) { status, expirationDate, error in
			completionHandler(status, expirationDate, error)
		}
	}
}

private func withOptionalLogHandler<T>(_ logHandler: ((String) -> Void)?, _ body: () -> T) -> T {
	if let logHandler {
		Zsign.setLogHandler(logHandler)
	}
	defer {
		if logHandler != nil {
			Zsign.setLogHandler(nil)
		}
	}
	return body()
}
