### Isolated user namespaces examples / demo

This repository contains some code examples for an "isolated user namespaces" concept.

The basis of it is a Linux kernel patches:
https://github.com/mihalicyn/linux/commits/isolated_userns

#### Demo

1. you need a VM that run's a patched Linux kernel version

2. It's convenient to open two terminal sessions to work from

Terminal 1 (host to "isolated" user namespace transition)
```
# first, you don't need to be root. Do everything as a regular user.

# clone this repo and build all tools
git clone https://github.com/mihalicyn/isolated-userns.git
cd isolated-userns
make

# spawn an isolated user namespace
#
# yes, you can also use
# $ unshare -Ur -m -p -f
# $ echo yes > /proc/self/isolated_uns
# but in this case your user namespace will be not fully
# isolated as a UID/GID 0 inside will be mapped to your current UID/GIDs.
#
# If you try to remove "-r" from unshare, it won't work properly too,
# as just after exec*() syscall all the capabilities will be dropped and
# you'll end up being absolutely unprivileged inside your user namespace which
# makes a whole setup absolutely useless.
#
# "go_isolated" tool does some tricks to make everything work fine (see source code for details)
#

./go_isolated unshare -m -p -f

# This also creates mount and pid namespaces

# Mount proc and tmpfs (example of a user namespace-local filesystems)
mount -t proc proc proc
mount -t tmpfs tmpfs tmpfs

# Now you can check UID/GIDs in the proc, for example:
ls -lan proc/
# and compare it with what you have in the host's proc:
ls -lan /proc

# Now let's play with tmpfs:
# - create two files with different UIDs
./setuid 123 touch tmpfs/uid123
./setuid 0 touch tmpfs/uid0
# - check what we have:
ls -lan tmpfs/

## Now the most important case. A filesystem shared from the host.

# Let's call:
sleep 1234

# Please, go to the Terminal session 2 instruction now.

# ok, you've got back.
# Let's create some files on the share:
./setuid 123 touch share/uid123
./setuid 0 touch share/uid0
ls -lan share/

# Now you can to to the Terminal session 1 again and check
# what you can see if you issue "ls -lan $HOSTDIRTOSHARE".

```

Terminal 2 (host)
```
# go to the repo directory
cd isolated-userns

# get our "sleep 1234" process PID:
SLEEP_PID=$(ps aux | grep "sleep 1234" | grep -v grep | awk '{ print $2 }')
REPODIR="/full/path/to/directory/with/git/repo/isolated-userns"
HOSTDIRTOSHARE="$REPODIR/share_from"

# enter mount namespace and create an idmapped (bind-)mount of the host's directory /host/dir/you/want/to/share
# you need to be root to do that!
nsenter -m -t $SLEEP_PID "$REPODIR"/mount-idmapped --map-mount=/proc/$SLEEP_PID/ns/user "$HOSTDIRTOSHARE" "$REPODIR"/share

kill -9 $SLEEP_PID

# please, get back to the terminal session 1
```