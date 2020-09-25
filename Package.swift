// swift-tools-version:5.0

import PackageDescription

let package = Package(
    name: "Impact",
    platforms: [.macOS(.v10_13), .iOS(.v12), .tvOS(.v12)],
    products: [
        .library(name: "Impact", targets: ["Impact"]),
    ],
    dependencies: [],
    targets: [
        .target(name: "Impact", dependencies: [], path: "Impact/", cSettings: [
          .headerSearchPath(""),
          .headerSearchPath("Utility"),
          .headerSearchPath("DWARF"),
          .headerSearchPath("Unwind"),
          .headerSearchPath("Monitoring"),
          .define("CURRENT_PROJECT_VERSION", to: "4")]
        ),
        // .testTarget(name: "ImpactTests", dependencies: ["Impact"], path: "ImpactTests/"),
    ]
)
