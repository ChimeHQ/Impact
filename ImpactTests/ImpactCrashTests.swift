//
//  ImpactCrashTests.swift
//  ImpactTests
//
//  Created by Matt Massicotte on 2019-09-17.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

import XCTest

class ImpactCrashTests: XCTestCase {
    lazy var testAppURL: URL = {
        let bundleURL = Bundle(for: type(of: self)).bundleURL
        let buildDirectoryURL = bundleURL.deletingLastPathComponent()

        return buildDirectoryURL.appendingPathComponent("ImpactTestMac.app")
    }()

    lazy var outputDirectory: URL = {
        return URL(fileURLWithPath: NSTemporaryDirectory(), isDirectory: true)
    }()

    func newOutputURL() -> URL {
        return outputDirectory.appendingPathComponent(UUID().uuidString, isDirectory: false)
    }

    func launchAppAndExecute(crash name: String) throws -> URL {
        let tempURL = newOutputURL()

        Swift.print("setting: \(tempURL.path)")
        let args = ["-output_path", tempURL.path, "-run", name, "-suppressReportCrash", "YES"]

        let app = try NSWorkspace.shared.launchApplication(at: testAppURL,
                                                           options: [.withoutActivation, .withoutAddingToRecents],
                                                           configuration: [.arguments: args])

        // There appear to be a races around launching, waiting for app.isFinishedLaunching,
        // and then waiting for app.isTerminated
        
        let terminationExpectation = keyValueObservingExpectation(for: app, keyPath: "isTerminated", expectedValue: true)

        wait(for: [terminationExpectation], timeout: 2.0)

        return tempURL
    }

    func readCrashData(at url: URL) -> [String] {
        let contents = try! Data(contentsOf: url)
        let stringContents = String(data: contents, encoding: .utf8)!

        return stringContents.components(separatedBy: "\n")
    }

    func testLaunchWithoutCrash() throws {
        let url = newOutputURL()

        let args = ["-output_path", url.path]
        let app = try NSWorkspace.shared.launchApplication(at: testAppURL,
                                                           options: [.withoutActivation, .withoutAddingToRecents],
                                                           configuration: [.arguments: args])


        app.terminate()

        let terminationExpectation = keyValueObservingExpectation(for: app, keyPath: "isTerminated", expectedValue: true)

        wait(for: [terminationExpectation], timeout: 2.0)

        let lines = readCrashData(at: url)

        XCTAssertFalse(lines.contains("hello from the mach exception handler"))
        XCTAssertFalse(lines.contains("[Thread:Crashed]"))
        XCTAssertTrue(lines.contains(where: { $0.hasPrefix("[Application] id: Y29tLmNoaW1laHEuSW1wYWN0VGVzdE1hYw==") }))
        XCTAssertTrue(lines.contains(where: { $0.hasPrefix("[Environment] platform: macOS") }))
    }

    func testCallAbort() throws {
        let url = try launchAppAndExecute(crash: "invokeAbort")
        let lines = readCrashData(at: url)

        XCTAssertTrue(lines.contains(where: { $0.hasPrefix("[Signal] signal: 0x6") }))
        XCTAssertTrue(lines.contains("[Thread:Crashed]"))

        try? FileManager.default.removeItem(at: url)
    }

    func testNullDereference() throws {
        let url = try launchAppAndExecute(crash: "nullDereference")
        let lines = readCrashData(at: url)

        XCTAssertTrue(lines.contains("hello from the mach exception handler"))
        XCTAssertTrue(lines.contains("[Thread:Crashed]"))
        
        try? FileManager.default.removeItem(at: url)
    }

    func testUncaughtNSException() throws {
        let url = try launchAppAndExecute(crash: "uncaughtNSException")
        let lines = readCrashData(at: url)

        XCTAssertTrue(lines.contains("hello from the mach exception handler"))
        XCTAssertTrue(lines.contains("[Exception] type: objc, name: QW5FeGNlcHRpb24=, message: c29tZXRoaW5nIGJhZCBoYXBwZW5lZA=="))

        try? FileManager.default.removeItem(at: url)
    }

    func testNonMainThreadUncaughtNSException() throws {
        let url = try launchAppAndExecute(crash: "nonMainThreadUncaughtNSException")
        let lines = readCrashData(at: url)

        XCTAssertTrue(lines.contains("hello from the mach exception handler"))
        XCTAssertTrue(lines.contains("[Exception] type: objc, name: QW5FeGNlcHRpb24=, message: c29tZXRoaW5nIGJhZCBoYXBwZW5lZA=="))

        try? FileManager.default.removeItem(at: url)
    }
}
