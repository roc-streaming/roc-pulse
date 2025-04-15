# Changelog

## [v0.0.8][v0.0.8] - 07 Oct 2024

* Fix PulseAudio download URL ([gh-31][gh-31])

[v0.0.8]: https://github.com/roc-streaming/roc-pulse/releases/tag/v0.0.8

[gh-31]: https://github.com/roc-streaming/roc-pulse/issues/31

## [v0.0.7][v0.0.7] - 14 Jun 2024

* Fix compatibility with roc-toolkit 0.4
* By default build roc-toolkit from master

[v0.0.7]: https://github.com/roc-streaming/roc-pulse/releases/tag/v0.0.7

## [v0.0.6][v0.0.6] - 28 May 2024

* Fix linker error

[v0.0.6]: https://github.com/roc-streaming/roc-pulse/releases/tag/v0.0.6

## [v0.0.5][v0.0.5] - 12 May 2024

* Add missing options for roc-toolkit 0.3 and upcoming 0.4
* Expand and improve documentation
* cmake: Fix linker error ([gh-29][gh-29])
* cmake: Add -DROC_REVISION

[v0.0.5]: https://github.com/roc-streaming/roc-pulse/releases/tag/v0.0.5

[gh-29]: https://github.com/roc-streaming/roc-pulse/issues/29

## [v0.0.4][v0.0.4] - 22 Nov 2023

* Switch to roc-toolkit 0.3
* Add RTCP endpoint and `local_control_port` / `remote_control_port` options
* Remove `resampler_profile=disable` option
* Disable OpenSSL when building roc-toolkit
* Suppress CMake warnings

[v0.0.4]: https://github.com/roc-streaming/roc-pulse/releases/tag/v0.0.4

## [v0.0.3][v0.0.3] - 10 Jul 2023

* Minor fix in tools detection in cmake

[v0.0.3]: https://github.com/roc-streaming/roc-pulse/releases/tag/v0.0.3

## [v0.0.2][v0.0.3] - 30 Dec 2022

* Fix undefined symbol to pthread ([gh-27][gh-27])
* Rename HOST to TOOLCHAIN_PREFIX in CMake
* Read default values for CMake options from env
* Update documentation

[v0.0.2]: https://github.com/roc-streaming/roc-pulse/releases/tag/v0.0.2

[gh-27]: https://github.com/roc-streaming/roc-pulse/issues/27

## [v0.0.1][v0.0.1] - 20 Dec 2022

* Initial release

[v0.0.1]: https://github.com/roc-streaming/roc-pulse/releases/tag/v0.0.1
