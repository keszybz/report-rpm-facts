/* SPDX-License-Identifier: LGPL-2.1-or-later */

#include <assert.h>
#include <stdlib.h>
#include <systemd/sd-varlink.h>
#include <systemd/sd-varlink-idl.h>

#include "common.h"
#include "metrics.h"
#include "report-packages.h"

static int vl_server(void) {
        _cleanup_(sd_varlink_server_unrefp) sd_varlink_server *s = NULL;
        int r;

        r = sd_varlink_server_new(&s, SD_VARLINK_SERVER_FD_PASSING_INPUT_STRICT);
        if (r < 0)
                return log_error_errno(r, "Failed to allocate varlink server object: %m");

        r = sd_varlink_server_set_info(s,
                                       "report-package-facts",
                                       "report-package-facts",
                                       PROJECT_VERSION,
                                       NULL);
        if (r < 0)
                return log_error_errno(r, "Failed to describe varlink server object: %m");

        r = metrics_add_to_varlink_server(s, vl_method_list_packages, vl_method_describe_packages);
        if (r < 0)
                return r;

        r = sd_varlink_server_loop_auto(s);
        if (r < 0)
                return log_error_errno(r, "Failed to run Varlink event loop: %m");

        return 0;
}

static int run(int argc, char **argv) {
        int r;

        if (argc != 1)
                return log_error_errno(EINVAL, "This program takes no arguments.");
        assert(argv);

        r = sd_varlink_invocation(SD_VARLINK_ALLOW_ACCEPT);
        if (r < 0)
                return log_error_errno(r, "Failed to check if invoked in Varlink mode: %m");
        if (r == 0)
                return log_error_errno(EINVAL, "This program can only run as a Varlink service.");

        return vl_server();
}

int main(int argc, char *argv[]) {
        return run(argc, argv) >= 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
