# SPDX-License-Identifier: LGPL-2.1-or-later

project('report-package-facts', 'c',
        version : files('meson.version'),
        license : 'LGPLv2+',
        default_options: [
                'c_std=c23',
                'warning_level=2',
        ],
)

librpm = dependency('rpm')
libsystemd = dependency('libsystemd')

includes = include_directories('src/shared')

conf = configuration_data()
conf.set_quoted('PROJECT_VERSION', meson.project_version())
conf.set('_GNU_SOURCE', 1)
conf.set('__SANE_USERSPACE_TYPES__', true)

config_h = configure_file(
        output : 'config.h',
        configuration : conf)

c_args = ['-include', 'config.h']

executable(
        'report-package-facts',
        sources : [
                'src/report-package-facts/main.c',
                'src/report-package-facts/report-packages.c',
                'src/shared/common.c',
                'src/shared/facts.c',
                'src/shared/varlink-io.systemd.Facts.c',
        ],
        c_args : c_args,
        include_directories : includes,
        dependencies : [
                librpm,
                libsystemd,
        ],
)
