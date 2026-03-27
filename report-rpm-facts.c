#include <rpm/rpmlib.h>
#include <rpm/rpmts.h>
#include <rpm/rpmdb.h>
#include <rpm/header.h>

int main() {
        rpmReadConfigFiles(NULL, NULL);
        rpmts ts = rpmtsCreate();
        Header h;

        // Iterate through every package in the database
        rpmdbMatchIterator mi = rpmtsInitIterator(ts, RPMDBI_PACKAGES, NULL, 0);

        while ((h = rpmdbNextIterator(mi)) != NULL) {
                rpmtd td = rpmtdNew();

                headerGet(h, RPMTAG_NEVRA, td, HEADERGET_EXT);

                const char *nevra = rpmtdGetString(td);
                printf("%s\n", nevra);

                rpmtdFree(td);
        }

        rpmdbFreeIterator(mi);
        rpmtsFree(ts);
        return 0;
}
