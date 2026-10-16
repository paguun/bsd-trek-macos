/*	$NetBSD: dumpgame.c,v 1.15 2009/08/12 08:54:54 dholland Exp $	*/

/*
 * Copyright (c) 1980, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
#ifndef lint
#if 0
static char sccsid[] = "@(#)dumpgame.c	8.1 (Berkeley) 5/31/93";
#else
__RCSID("$NetBSD: dumpgame.c,v 1.15 2009/08/12 08:54:54 dholland Exp $");
#endif
#endif /* not lint */

/* macOS port: ASLR-independent saves. Same-build/architecture only.
 * The original game structures and gameplay are unchanged. Runtime pointers
 * are represented by event indices on disk and rebound after loading.
 */
#include <stdio.h>
#include <err.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "trek.h"

struct saved_state {
    struct Ship_struct ship;
    struct Now_struct now;
    struct Param_struct param;
    struct Etc_struct etc;
    struct Game_struct game;
    char sectors[NSECTS][NSECTS];
    struct quad quadrants[NQUADS][NQUADS];
    struct Move_struct move;
    struct event events[MAXEVENTS];
};

struct save_header {
    char magic[8];
    uint32_t byte_order;
    uint32_t sizes[10];
    int32_t now_events[NEVENTS];
    int32_t snapshot_events[NEVENTS];
};

static void
make_header(struct save_header *h)
{
    memset(h, 0, sizeof *h);
    memcpy(h->magic, "TREKMAC3", 8);
    h->byte_order = UINT32_C(0x01020304);
    h->sizes[0] = sizeof(struct saved_state);
    h->sizes[1] = sizeof Ship;
    h->sizes[2] = sizeof Now;
    h->sizes[3] = sizeof Param;
    h->sizes[4] = sizeof Etc;
    h->sizes[5] = sizeof Game;
    h->sizes[6] = sizeof Sect;
    h->sizes[7] = sizeof Quad;
    h->sizes[8] = sizeof Move;
    h->sizes[9] = sizeof Event;
}

static int
encode_events(struct Now_struct *n, int32_t indices[NEVENTS])
{
    int i, j;
    for (i = 0; i < NEVENTS; ++i) {
        indices[i] = -1;
        if (n->eventptr[i] != NULL) {
            for (j = 0; j < MAXEVENTS; ++j)
                if (n->eventptr[i] == &Event[j])
                    break;
            if (j == MAXEVENTS)
                return -1;
            indices[i] = j;
        }
        n->eventptr[i] = NULL;
    }
    return 0;
}

static int
valid_indices(const int32_t indices[NEVENTS])
{
    int i;
    for (i = 0; i < NEVENTS; ++i)
        if (indices[i] < -1 || indices[i] >= MAXEVENTS)
            return 0;
    return 1;
}

static void
decode_events(struct Now_struct *n, const int32_t indices[NEVENTS])
{
    int i;
    for (i = 0; i < NEVENTS; ++i)
        n->eventptr[i] = indices[i] < 0 ? NULL : &Event[indices[i]];
}

/* Handle short reads/writes and EINTR without accepting partial files. */
static int
transfer(int fd, void *buffer, size_t count, int writing)
{
    unsigned char *p = buffer;
    while (count != 0) {
        ssize_t done = writing ? write(fd, p, count) : read(fd, p, count);
        if (done < 0 && errno == EINTR)
            continue;
        if (done <= 0) {
            if (done == 0)
                errno = EIO;
            return -1;
        }
        p += done;
        count -= (size_t)done;
    }
    return 0;
}

void
dumpgame(int v __unused)
{
    struct saved_state s;
    struct save_header h;
    struct Now_struct snap;
    size_t offset = sizeof Quad + sizeof Event;
    char temp[] = "trek.dump.tmp.XXXXXX";
    int fd, error;

    memset(&s, 0, sizeof s);
    make_header(&h);
    s.ship = Ship; s.now = Now; s.param = Param; s.etc = Etc;
    s.game = Game; s.move = Move;
    memcpy(s.sectors, Sect, sizeof Sect);
    memcpy(s.quadrants, Quad, sizeof Quad);
    memcpy(s.events, Event, sizeof Event);
    s.ship.shipname = NULL;
    if (encode_events(&s.now, h.now_events) != 0)
        goto bad_state;
    if (Game.snap) {
        memcpy(&snap, s.etc.snapshot + offset, sizeof snap);
        if (encode_events(&snap, h.snapshot_events) != 0)
            goto bad_state;
        memcpy(s.etc.snapshot + offset, &snap, sizeof snap);
    }
    fd = mkstemp(temp);
    if (fd < 0) {
        warn("cannot save `trek.dump'");
        return;
    }
    error = transfer(fd, &h, sizeof h, 1) ||
        transfer(fd, &s, sizeof s, 1) || fsync(fd) != 0;
    if (close(fd) != 0)
        error = 1;
    if (!error && rename(temp, "trek.dump") == 0) {
        printf("Game saved to trek.dump\n");
        return;
    }
    warn("cannot save `trek.dump'");
    unlink(temp);
    return;
bad_state:
    printf("cannot save: invalid event state\n");
}

int
restartgame(void)
{
    struct saved_state s;
    struct save_header h, expected;
    struct Now_struct snap;
    size_t offset = sizeof Quad + sizeof Event;
    char extra;
    int fd, error;
    ssize_t trailing;

    fd = open("trek.dump", O_RDONLY);
    if (fd < 0)
        goto failed;
    make_header(&expected);
    error = transfer(fd, &h, sizeof h, 0);
    if (!error)
        error = memcmp(h.magic, expected.magic, sizeof h.magic) != 0 ||
            h.byte_order != expected.byte_order ||
            memcmp(h.sizes, expected.sizes, sizeof h.sizes) != 0 ||
            !valid_indices(h.now_events) ||
            !valid_indices(h.snapshot_events);
    if (!error)
        error = transfer(fd, &s, sizeof s, 0);
    if (!error) {
        do {
            trailing = read(fd, &extra, 1);
        } while (trailing < 0 && errno == EINTR);
        error = trailing != 0 ||
            (s.ship.ship != ENTERPRISE && s.ship.ship != QUEENE);
    }
    close(fd);
    if (error)
        goto failed;

    /* Commit only after the entire file was read and its format checked. */
    decode_events(&s.now, h.now_events);
    if (s.game.snap) {
        memcpy(&snap, s.etc.snapshot + offset, sizeof snap);
        decode_events(&snap, h.snapshot_events);
        memcpy(s.etc.snapshot + offset, &snap, sizeof snap);
    }
    s.ship.shipname = s.ship.ship == ENTERPRISE ? "Enterprise" : "Fairie Queene";
    Ship = s.ship; Now = s.now; Param = s.param; Etc = s.etc;
    Game = s.game; Move = s.move;
    memcpy(Sect, s.sectors, sizeof Sect);
    memcpy(Quad, s.quadrants, sizeof Quad);
    memcpy(Event, s.events, sizeof Event);
    printf("Game restored from trek.dump\n");
    return 0;
failed:
    printf("cannot restart: missing, incompatible, or incomplete trek.dump\n");
    return 1;
}
