/* SPDX-License-Identifier: LGPL-2.1-or-later */

#include <assert.h>
#include <ctype.h>
#include <rpm/header.h>
#include <rpm/rpmdb.h>
#include <rpm/rpmlib.h>
#include <rpm/rpmts.h>

#include "common.h"
#include "report-packages.h"

#define FACT_PREFIX "org.systemd.Packages"

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
                rpmtd td = rpmtdNew(), td2 = rpmtdNew();

                if (!headerGet(h, RPMTAG_NAME, td, 0))
                        return -EIO;

                if (!headerGet(h, RPMTAG_NEVRA, td2, HEADERGET_EXT))
                        return -EIO;

                const char *name = rpmtdGetString(td);
                const char *nevra = rpmtdGetString(td2);

                assert(nevra);
                assert(nevra[0]);
                
                /* nevra should have the format <name>-[<epoch>:]<version>-<release>… */
                assert(nevra[strlen(name)] == '-');
                const char *evra = nevra + strlen(name) + 1;

                _cleanup_free_ char *purl = strjoin("pkg:rpm//", name, "@", evra);
                if (!purl)
                        return -ENOMEM;

                rpmtdFree(td);
                rpmtdFree(td2);

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
