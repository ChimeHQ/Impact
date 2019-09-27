# Impact

Impact is a crash reporting library for Apple platforms. It just detects and records information about crashes. Impact isn't a full crash monitoring system, but it could be the core of one. Its design goals are:

* Improve understanding of crash reporting systems
* Support for all Apple platforms
* Reliablity
* Accuracy
* Simplicity
* Fun

## Isn't Crash Reporting a Solved Problem?

In-process crash reporting is just terrible. The mechanisms available for crash event detection, UNIX signals and Mach exceptions, are complex, buggy, and are not capable of capturing all kinds of failures. On top of that, the enviroment in which a crash reporter needs to run is extraordinarily hostile. It's just messy business.

Apple has had all of the device-side pieces in place to produce a world-class crsah reporting system for a long time. While they have the reporting side down, the developer experience (analysis, presentation, investigative tools) leave a lot to be desired. This has kept 3rd-party reporting services essential for the vast majority of app developers. Their offerings also don't work for macOS apps outside the App Store, which is disappointing.

My sincere hope is that Apple addresses these limitations so we can all stop this foolishness once and for all.

But, for now, in-process reporting is a necessary component for most developers. And, by no means is it a solved problem. There are many trade-offs and design decisions that dramatically affect the qualities of a reporting system. Understanding those trade-offs, and being explicit about decisions is one of the goals of this project.

Also, crash reporting is just a fun and facinating problem. It tends to be very commonly used and very poorly understood. I think the Apple development community could benefit a lot from a more in-depth understanding of the area.

## Can I Use Impact in my App?

Yes you can. But, you must keep in mind that Impact **only** captures information about crash events. It does not have any facilities for transmitting those events back to you, translating those events into human-readable versions, or processing them into groups.

Impact also is a work-in-progress. Here's the plan:

| Feature            | Supported |
| -------------------|:---------:|
| UNIX signals | ✅ |
| Mach Exceptions | ✅ |
| Basic Thread details | - |
| Binary image information | - |
| Frame pointer-based stack traces | - |
| Compact Unwind support | - |
| DWARF CFI support | - |

You'll probably want to wait until at least you have some stack traces before considering using Impact for real.

## Relationship to Crashlytics

I worked at [Crashlytics](https://firebase.google.com/products/crashlytics) for many years. During my time there, I briefly worked with [PLCrashReporter](https://www.plcrashreporter.org) before starting from scratch and building a completely custom system. I spent a considerable amount of time there analyzing and understanding the failure modes of in-process crash reporting. That work shaped most of the design of the Crashlytics SDK, though things might have changed since I left.

Impact does share many of those design philosophies. Primarily because I believe those core design concepts are the best approach. It's also hard to unsee solutions, sometimes. However, they have chosen to keep their SDK closed source, and I want to respect that. While I haven't had direct access the code for years, I'll still do my best to keep non-public information that way.

## Incorporating Impact

If you want to use the project as part of a crash reporting service (ie accept data from apps you do not write yourself), you are more than welcome. However, you **must** let the maintainers of this project know before you ship. If you just want to use Impact for your own app, go for it! You don't have to get in touch. But, it would be cool :)

## Contributing

It would be wonderful to see contributions. If you'd like to work on something, the safest bet is to open an issue or PR first. That way, we can discuss the changes before you spend too much time working.

There is aboslutely no experience/knowledge requirement. Interest is all you need, I am happy to help.

## Suggestions or Feedback

We'd love to hear from you! Get in touch via [twitter](https://twitter.com/chimehq), an issue, or a pull request.

Please note that this project is released with a [Contributor Code of Conduct](CODE_OF_CONDUCT.md). By participating in this project you agree to abide by its terms.
