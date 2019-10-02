//
//  AppDelegate.swift
//  ImpactTestMac
//
//  Created by Matt Massicotte on 2019-09-17.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

import Cocoa
import Impact

@NSApplicationMain
class AppDelegate: NSObject, NSApplicationDelegate {
    var window: NSWindow!
    var viewController: ViewController!

    func applicationDidFinishLaunching(_ aNotification: Notification) {
        UserDefaults.standard.register(defaults: [
            "NSApplicationCrashOnExceptions": true,
        ])

        let path = UserDefaults.standard.string(forKey: "output_path") ?? "/tmp/impact";
        let url = URL(fileURLWithPath: path, isDirectory: false)

        ImpactMonitor.shared().suppressReportCrash = UserDefaults.standard.bool(forKey: "suppressReportCrash")
        ImpactMonitor.shared().start(with: url, identifier: "abc123")

        self.viewController = ViewController()
        self.window = NSWindow(contentViewController: viewController)

        window.setContentSize(NSSize(width: 400, height: 300))
        window.makeKeyAndOrderFront(self)

        guard let crash = crashFromCommandLineArgument() else {
            return
        }

        DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
            crash.execute()
        }
    }

    func applicationWillTerminate(_ aNotification: Notification) {
    }
}

extension AppDelegate {
    func crashFromCommandLineArgument() -> CrashInvocation? {
        let crashName = UserDefaults.standard.string(forKey: "run")

        switch crashName {
        case "invokeAbort"?:
            return InvokeAbort()
        case "nullDereference"?:
            return NullDereference()
        case "uncaughtNSException"?:
            return UncaughtNSException()
        case "nonMainThreadUncaughtNSException"?:
            return NonMainThreadUncaughtNSException()
        default:
            return nil
        }
    }
}
