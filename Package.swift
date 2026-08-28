// swift-tools-version: 5.8

import PackageDescription

let package = Package(
	name: "Zsign",
	platforms: [
		.iOS(.v12),
		.macOS(.v10_15),
		.tvOS(.v12),
		.watchOS(.v8),
	],
	products: [
		.library(name: "Zsign", targets: ["Zsign"]),
		.library(name: "ZsignSwift", targets: ["ZsignSwift"]),
		.library(name: "ZsignIPAX", targets: ["ZsignIPAX"]),
	],
	dependencies: [
		.package(url: "https://github.com/krzyzanowskim/OpenSSL", from: "3.3.3001"),
	],
	targets: [
		.target(
			name: "minizip",
			path: "Core/third-party/minizip",
			sources: ["ioapi.c", "zip.c", "unzip.c"],
			publicHeadersPath: ".",
			cSettings: [
				.headerSearchPath("."),
			],
			linkerSettings: [
				.linkedLibrary("z"),
			]
		),
		.target(
			name: "Zsign",
			dependencies: [
				.product(name: "OpenSSL", package: "OpenSSL"),
				"minizip",
			],
			path: ".",
			exclude: [
				"Sources/ZsignIPAX",
				"Sources/ZsignIPAXBridgeCore",
				"Tests/ZsignIPAXTests",
				"Tests",
				"Core/third-party",
				"PENDING_IPAX_FEATURES.md",
			],
			sources: [
				"Core/archo.cpp",
				"Core/bundle.cpp",
				"Core/certcheck.cpp",
				"Core/macho.cpp",
				"Core/metadata.cpp",
				"Core/openssl.cpp",
				"Core/signing.cpp",
				"Core/common/fs.cpp",
				"Core/common/json.cpp",
				"bridge/archive_overlay.cpp",
				"bridge/archive_zip_progress.cpp",
				"bridge/archive_cancel.cpp",
				"bridge/fs_ipa_junk.cpp",
				"bridge/verify_archo.cpp",
				"bridge/verify_macho_map.cpp",
				"bridge/verify_signed_bundle.cpp",
				"bridge/log_overlay.cpp",
				"bridge/i18n/zlog_i18n.cpp",
				"Core/common/sha.cpp",
				"Core/common/timer.cpp",
				"Core/common/util.cpp",
				"bridge/zsign.mm",
				"bridge/extended_api.mm",
				"bridge/check_cert_loader.cpp",
				"bridge/macho_dylib_overlay.cpp",
			],
			publicHeadersPath: "bridge/include",
			cxxSettings: [
				.headerSearchPath("Core"),
				.headerSearchPath("Core/common"),
				.headerSearchPath("bridge"),
				.headerSearchPath("bridge/include"),
				.headerSearchPath("bridge/i18n"),
			],
			linkerSettings: [
				.linkedFramework("Foundation"),
				.linkedLibrary("z"),
			]
		),
		.target(
			name: "ZsignSwift",
			dependencies: ["Zsign"],
			path: "Sources",
			exclude: ["ZsignIPAX", "ZsignIPAXBridgeCore"],
		),
		.target(
			name: "ZsignIPAXBridgeCore",
			dependencies: [
				"Zsign",
				.product(name: "OpenSSL", package: "OpenSSL"),
			],
			path: "Sources/ZsignIPAXBridgeCore",
			publicHeadersPath: "include",
			cxxSettings: [
				.headerSearchPath("include"),
			],
			linkerSettings: [
				.linkedFramework("Foundation"),
			]
		),
		.target(
			name: "ZsignIPAX",
			dependencies: ["ZsignIPAXBridgeCore", "Zsign"],
			path: "Sources/ZsignIPAX",
		),
		.testTarget(
			name: "ZsignIPAXTests",
			dependencies: ["ZsignIPAX"],
			path: "Tests/ZsignIPAXTests"
		),
	],
	cxxLanguageStandard: .cxx17
)
