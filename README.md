# Report "rpm facts"

This is a PoC service that implements a provider of the new systemd "facts" interface.
It returns a list of rpms currently installed on the system as
[PURLs](https://packageurl.org/docs/purl/introduction).

The service is written in C and uses sd-varlink.

## To build

`meson setup build && ninja -C build`

## To test

`varlinkctl call --more ./build/report-package-facts io.systemd.Facts.List {}`
