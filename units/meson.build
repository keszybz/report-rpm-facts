# SPDX-License-Identifier: LGPL-2.1-or-later

install_data(
        'report-package-facts.socket',
        install_dir : systemdunitdir,
)


configure_file(
        input : 'report-package-facts@.service.in',
        output : 'report-package-facts@.service',
        configuration : conf,
        install : true,
        install_dir : systemdunitdir,
)
