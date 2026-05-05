# Report "rpm metrics"

This is a PoC service that implements a provider of the new systemd "metrics" interface.
It returns a list of rpms currently installed on the system as
[PURLs](https://packageurl.org/docs/purl/introduction).

The service is written in C and uses sd-varlink.

## To build

`meson setup build && ninja -C build`

## To test

`varlinkctl call --more ./build/report-package-facts io.systemd.Metrics.List {}`


## License

This program is licensed under the terms of the GNU Lesser General Public License,
Version 2.1 or later.
