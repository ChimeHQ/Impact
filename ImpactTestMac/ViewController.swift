//
//  ViewController.swift
//  ImpactTestMac
//
//  Created by Matt Massicotte on 2019-09-17.
//  Copyright © 2019 Chime Systems Inc. All rights reserved.
//

import Cocoa

class ViewController: NSViewController {
    private let tableView: NSTableView
    private let crashInvocations: [CrashInvocation]

    init() {
        self.tableView = NSTableView()
        self.crashInvocations = [
            InvokeAbort(),
            NullDereference(),
            UncaughtNSException(),
            NonMainThreadUncaughtNSException(),
        ]

        super.init(nibName: nil, bundle: nil)
    }

    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }

    override func loadView() {
        let column = NSTableColumn(identifier: NSUserInterfaceItemIdentifier("column"))
        column.width = 100.0

        tableView.addTableColumn(column)
        tableView.delegate = self
        tableView.dataSource = self
        tableView.headerView = nil
        tableView.allowsTypeSelect = false
        tableView.allowsMultipleSelection = false
        tableView.allowsColumnSelection = false
        tableView.target = self
        tableView.doubleAction = #selector(performCrash(_:))

        let scrollView = NSScrollView()

        scrollView.documentView = tableView
        scrollView.hasVerticalScroller = true
        scrollView.hasHorizontalScroller = true

        self.view = scrollView
    }

    @objc func performCrash(_ sender: Any?) {
        let row = tableView.selectedRow

        let invocation = crashInvocations[row]

        invocation.execute()
    }
}

extension ViewController: NSTableViewDelegate {
}

extension ViewController: NSTableViewDataSource {
    func numberOfRows(in tableView: NSTableView) -> Int {
        return crashInvocations.count
    }

    private func makeNewRowView() -> NSTextField {
        let textField = NSTextField()

        textField.drawsBackground = false
//        textField.backgroundColor = NSColor.white
        textField.isBezeled = false
        textField.allowsDefaultTighteningForTruncation = true
        textField.isEditable = false
        textField.maximumNumberOfLines = 1
        textField.cell?.truncatesLastVisibleLine = true

        return textField
    }

    func tableView(_ tableView: NSTableView, viewFor tableColumn: NSTableColumn?, row: Int) -> NSView? {
        let reusedView = tableView.makeView(withIdentifier: NSUserInterfaceItemIdentifier("column"), owner: self)
        let cellView = (reusedView as? NSTextField) ?? makeNewRowView()

        let invocation = crashInvocations[row]

        cellView.stringValue = invocation.name

        return cellView
    }
}
