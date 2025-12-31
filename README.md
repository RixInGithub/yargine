<div align="center">
	<h1>yargine!</h1>
	<blockquote><i>a game engine for the newgens, i guess.</i></blockquote>
</div>
yargine is a huge work in progress game engine built in pure c, using binaryen, readline and native posix libs.

# build?
for now, most compatability goes on unix-like systems, however, windows (and maybe macos) compatability is in the works.

requirements:

- readline (should be preinstalled on modern distros)
- jorkdir (submodule)
- cwalk (submodule)

planned usage: [binaryen](https://github.com/WebAssembly/binaryen/)

before building, run `git submodule update --init --recursive` just in case or smth idk

for linux, just run `./build` and it'll automatically build for you and launch yargine! (unless if you specify any arg)

# roadmap?
- lyarg
- wasm export (with binaryen)
- native linux/windows export (with sdl?)