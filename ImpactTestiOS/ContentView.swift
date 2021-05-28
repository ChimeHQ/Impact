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
            Text("Precondition Failure").onTapGesture {
                precondition(false)
            }
            Text("Uncaught Exception").onTapGesture {
                _ = NSArray()[0]
            }
        }
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
