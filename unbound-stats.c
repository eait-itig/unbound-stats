/* */

/*
 * Copyright (c) 2024 The University of Queensland
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * This is a helper program that's supposed to make it possible to
 * only read unbound stats by an otherwise unprivileged user, without
 * making it possible to push other control commands to the daemon.
 */

#include <stdio.h>
#include <unistd.h>
#include <err.h>

/*
 * echo -n "UBCT1 stats_noreset\n" | \
 * nc -c \
 *   -C /var/unbound/etc/unbound_control.pem \
 *   -K /var/unbound/etc/unbound_control.key \
 *   -R /var/unbound/etc/unbound_server.pem \
 *   -e unbound localhost 8953
 */

#define NETCAT		"nc"
#define NETCATBIN	"/usr/bin/" NETCAT

#define UBCT_CRT	"/var/unbound/etc/unbound_control.pem"
#define UBCT_KEY	"/var/unbound/etc/unbound_control.key"
#define UBCT_CA		"/var/unbound/etc/unbound_server.pem"
#define UBCT_SUBJECT	"unbound"

#define UBCT_HOST	"localhost"
#define UBCT_PORT	"8953"

#define UBCT_CMD	"UBCT1 stats_noreset\n"

static const char ubct_cmd[] = UBCT_CMD;
static const size_t ubct_cmdlen = sizeof(ubct_cmd) - 1; /* less the nul */

int
main(int argc, char *argv[])
{
	char *ncargv[] = {
	    NETCAT, "-c",
	    "-C", UBCT_CRT, "-K", UBCT_KEY,
	    "-R", UBCT_CA,
	    "-e", UBCT_SUBJECT,
	    UBCT_HOST, UBCT_PORT,
	    NULL
	};
	int ncstdin[2];
	ssize_t rv;

	if (pipe(ncstdin) == -1)
		err(1, "pipe");

	switch (fork()) {
	case -1:
		err(1, "fork");
	case 0: /* child */
		close(ncstdin[0]); /* not really needed, but nice */
		ncstdin[0] = -1;
		break;
	default: /* parent */
		if (dup2(ncstdin[0], 0) == -1)
			err(1, "%s stdin", NETCAT);
		if (closefrom(4) == -1)
			err(1, "%s closefrom", NETCAT);

		execv(NETCATBIN, ncargv);
		/* we only get here if we errored out */
		err(1, "%s", NETCAT);
		/* NOTREACHED */
	}

	rv = write(ncstdin[1], ubct_cmd, ubct_cmdlen);
	if (rv == -1)
		err(1, "write command");
	if ((size_t)rv != ubct_cmdlen)
		errx(1, "wrote %zd of %zu bytes", rv, ubct_cmdlen);

	return (0);
}
