<div align="center">
	<img src="y.svg" width="64">
	<h1>yargine!</h1>
	<blockquote><i>a game engine for the newgens, i guess.</i></blockquote>
</div>
yargine is a huge work in progress game engine built in pure c, using binaryen, readline and native posix libs.

## build?
for now, most compatability goes on unix-like systems, however, windows (and maybe macos) compatability is in the works.

requirements:

- readline (should be preinstalled on modern distros)
- jorkdir (submodule)
- cwalk (submodule)

planned usage: [binaryen](https://github.com/WebAssembly/binaryen/)

before building, run `git submodule update --init --recursive` just in case or smth idk

for linux, just run `./build` and it'll automatically build for you and launch yargine! (unless if you specify any arg)

also on linux you can build a `yargine.AppImage`! doesn't that sound exciting? sure it does!
just run `./mkappimage` and it'll create and launch an `AppImage`!

## build online edition?
this is definitely going to be an unix-like exclusive feature of yargine, yargine online!

have an old amiga laptop that doesnt have posix? ok but does it have ssh? probably? well than you can use yargine online!
made for the people out there who have unsupported platforms which can't build yargine!

to build yargine online, you can run `YONLINE=1 ./build`, that will make an env var for ./build specifically to build the online edition instead of the regular edition.

after, you can go check out your `~/.ssh/id_ed25519.pub`, copy the whole file (prob smth like `ed25519` followed by some ssh gibberish and some sort of identity (eg email, dip@shit, etc))
then go edit `~/.ssh/authorized_keys` and edit it to the following:

```
command="/path/to/yargine/yargine.x86_64",no-port-forwarding,no-X11-forwarding,no-agent-forwarding
```

then add a space & paste in your `~/.ssh/id_ed25519.pub`, it should look smth like this:

```
command="/path/to/yargine/yargine.x86_64",no-port-forwarding,no-X11-forwarding,no-agent-forwarding ed25519 AAAA... dip@shit
```

congrats! now make sure sshd is up and running (`sudo systemctl start ssh && sudo systemctl enable ssh`) and connect with `ssh localhost`!

## roadmap?
- yargine online (yargine on ssh)
- lyarg
- wasm export (with binaryen)
- native linux/windows export (with sdl?)