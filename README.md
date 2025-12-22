<div align="center">
	<h1>yargine!</h1>
	<blockquote><i>a game engine for the newgens, i guess.</i></blockquote>
</div>
yargine is a huge work in progress game engine built in pure c, using binaryen, readline and native posix libs.

# build?
for now, most compatability goes on unix-like systems, however, windows (and maybe macos) compatability is in the works.

requirements:

- readline (should be preinstalled on modern distros)
- [binaryen](https://github.com/WebAssembly/binaryen/releases/)
- jorkdir (submodule)
- cwalk (submodule)

before building, run `git submodule update` just in case

for linux, just run `./build` and it'll automatically build for you and launch yargine!

# roadmap?
- base project render screen
- yargL
- wasm export (with binaryen)
- [castle](https://castle.xyz) export
- native linux/windows export (with sdl?)
