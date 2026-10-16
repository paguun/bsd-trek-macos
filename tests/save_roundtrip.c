/* Regression checks for the macOS save-file adaptation; not game rules. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "trek.h"

int
main(int argc, char **argv)
{
    struct Now_struct snap;
    int i;
    size_t offset = sizeof Quad + sizeof Event;
    assert(argc == 2);
    if (strcmp(argv[1], "save") == 0) {
        Ship.ship = ENTERPRISE;
        Ship.shipname = "Enterprise";
        Ship.energy = 4321;
        Ship.quadx = 3;
        Now.date = 2700.25;
        Game.snap = 1;
        for (i = 0; i < NEVENTS; ++i)
            Now.eventptr[i] = i == 0 ? NULL : &Event[i + 1];
        memset(&snap, 0, sizeof snap);
        snap.date = 2699.75;
        snap.eventptr[E_SNAP] = &Event[24];
        memcpy(Etc.snapshot + offset, &snap, sizeof snap);
        Event[24].evcode = E_SNAP;
        Event[24].date = 2701.0;
        Quad[3][2].klings = 4;
        Sect[2][3] = KLINGON;
        dumpgame(0);
        return 0;
    }
    if (strcmp(argv[1], "reject") == 0) {
        Ship.energy = 8765;
        Now.date = 123.0;
        assert(restartgame() == 1);
        assert(Ship.energy == 8765 && Now.date == 123.0);
        return 0;
    }
    assert(restartgame() == 0);
    assert(Ship.energy == 4321 && Ship.quadx == 3);
    assert(strcmp(Ship.shipname, "Enterprise") == 0);
    assert(Now.date == 2700.25 && Now.eventptr[0] == NULL);
    for (i = 1; i < NEVENTS; ++i)
        assert(Now.eventptr[i] == &Event[i + 1]);
    memcpy(&snap, Etc.snapshot + offset, sizeof snap);
    assert(snap.date == 2699.75 && snap.eventptr[E_SNAP] == &Event[24]);
    assert(Event[24].date == 2701.0 && Event[24].evcode == E_SNAP);
    assert(Quad[3][2].klings == 4 && Sect[2][3] == KLINGON);
    puts("save round-trip and event pointer rebinding: OK");
    return 0;
}
