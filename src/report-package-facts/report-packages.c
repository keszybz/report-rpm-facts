/* SPDX-License-Identifier: LGPL-2.1-or-later */

#include <assert.h>
#include <ctype.h>
#include <rpm/header.h>
#include <rpm/rpmdb.h>
#include <rpm/rpmlib.h>
#include <rpm/rpmts.h>

#include "common.h"
#include "report-packages.h"

#define FACT_PREFIX "io.systemd.Packages"

static const char* map_vendor(const char *v) {
        if (!v)
                return NULL;
        if (streq(v, "Fedora Project"))
                return "fedora";
        return NULL;
}

static int packages_generate(FactFamilyContext *context, _unused_ void *userdata) {
        int r;

        assert(context);

        /* Initialize rpmdb state */
        rpmReadConfigFiles(NULL, NULL);
        rpmts ts = rpmtsCreate();

        /* Iterate through every package in the database */
        rpmdbMatchIterator mi = rpmtsInitIterator(ts, RPMDBI_PACKAGES, NULL, 0);

        Header h;
        while ((h = rpmdbNextIterator(mi)) != NULL) {
                rpmtd td = rpmtdNew(), td2 = rpmtdNew(), td3 = rpmtdNew(), td4 = rpmtdNew(), td5 = rpmtdNew(), td6 = rpmtdNew();

                if (!headerGet(h, RPMTAG_NAME, td, 0))
                        return -EIO;

                if (!headerGet(h, RPMTAG_VERSION, td2, 0))
                        return -EIO;

                if (!headerGet(h, RPMTAG_RELEASE, td3, 0))
                        return -EIO;

                const char *name = rpmtdGetString(td);
                const char *version = rpmtdGetString(td2);
                const char *release = rpmtdGetString(td3);

                const char *vendor = NULL;
                if (headerGet(h, RPMTAG_VENDOR, td4, HEADERGET_EXT))
                        vendor = map_vendor(rpmtdGetString(td4));

                const char *arch = NULL;
                if (headerGet(h, RPMTAG_ARCH, td5, 0))
                        arch = rpmtdGetString(td5);

                uint64_t epoch = 0;
                char epochstr[sizeof("18446744073709551616")] = "";
                if (headerGet(h, RPMTAG_EPOCH, td6, HEADERGET_EXT))
                        epoch = rpmtdGetNumber(td6);
                if (epoch > 0)
                        snprintf(epochstr, sizeof(epochstr), "%lu", epoch);

                _cleanup_free_ char *purl = strjoin(
                                "pkg:rpm/", vendor,
                                "/", name, "@", version, "-", release,
                                arch ? "?arch=" : "", arch,
                                epoch > 0 ? "?epoch=" : "", epochstr);
                if (!purl)
                        return -ENOMEM;

                rpmtdFree(td);
                rpmtdFree(td2);
                rpmtdFree(td3);
                rpmtdFree(td4);
                rpmtdFree(td5);
                rpmtdFree(td6);

                r = fact_build_send_string(context, /* object= */ NULL, purl);
                if (r < 0)
                        return r;
        }

        rpmdbFreeIterator(mi);
        rpmtsFree(ts);
        return 0;
}

static const FactFamily fact_family_table[] = {
        /* Keep facts ordered alphabetically */
        {
                .name = FACT_PREFIX ".Package",
                .description = "Installed system package (as purl)",
                .generate = packages_generate,
        },
        {}
};

int vl_method_list_packages(sd_varlink *link, sd_json_variant *parameters, sd_varlink_method_flags_t flags, void *userdata) {
        return facts_method_list(fact_family_table, link, parameters, flags, userdata);
}

int vl_method_describe_packages(sd_varlink *link, sd_json_variant *parameters, sd_varlink_method_flags_t flags, void *userdata) {
        return facts_method_describe(fact_family_table, link, parameters, flags, userdata);
}
