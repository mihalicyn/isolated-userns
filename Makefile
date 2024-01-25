all: mount-idmapped setuid go_isolated dirs

dirs:
	mkdir share_from proc share tmpfs

%: %.c
	$(CC) $< -o $@

clean:
	rm -f mount-idmapped setuid go_isolated
	rm -r share_from/* || true
	rmdir share_from proc share tmpfs