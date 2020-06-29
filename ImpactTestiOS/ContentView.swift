//
//  ContentView.swift
//  ImpactTestiOS
//
//  Created by Matt Massicotte on 2020-06-24.
//  Copyright © 2020 Chime Systems Inc. All rights reserved.
//

import SwiftUI

struct ContentView: View {
    var body: some View {
        List {
            Text("Hello World").onTapGesture {
                fatalError()
            }
            Text("Hello World")
            Text("Hello World")
        }
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
