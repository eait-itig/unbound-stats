# unbound-stats

unbound-stats is intended to allow an otherwise unprivileged user to
read stats from an unbound daemon. It is the effectively the same as
`unbound-control stats_noreset`.

The trick^Wway it works is by being installed with the set-group-id on exec (setgid) bit with a group that is able to read the TLS certificates and keys necessary to communicate with the unbound daemon. Internally it uses `nc` to communicate with unbound, and unconditionally sends a stats_noreset command.

## Notes

- A lot of the configuration (eg, paths to TLS cerst/keys) is hardcoded
  in the program.
- This builds and works on OpenBSD with a fairly vanilla unbound.conf.
