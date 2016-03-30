# Game!
It's about time we make ourselves a game!

This is a multiplayer game so there are a few build artifacts.
1) Game client. This is what the user downloads and plays.
2) Game server. This is what the clients connect to to play the game.

The server for now is just 1 executable that the clients all connect to. This is definately going to need to scale into something larger once the game goes into alpha / beta that allows us to do matchmaking with multiple servers to run game instances on.

## Dependencies
### Linux
On linux you need to install a few packages.

* `sudo apt-get update`
* `sudo apt-get install build-essential libsdl2-dev libsdl2-net-dev -y`

### OSX
On osx everything needed is packaged in this repo.

### Windows
Still tbd, no windows support yet.

## Local Development
* Build client. `make client`
* Build server. `make server`

The makefile switches on the os we're running on so on osx `make client` will build the osx client which will be at `./build/client/osx/Game.app`

To run locally first start the local server `./build/server/game` and then one or more instances of the client `./build/client/osx/Game.app/Contents/MacOS/Game` (note, you usually want to run the game executable like this instead of `open Game.app` because that way you'll get logging to the console and can run more than one version.

## Deploying Releases
All of the scripts default to the correct setup to run and test locally but they take some optional parameters that are important for building releases. I think most of that stuff is gonna be handled by travis.

This is tbd too but some stuff we probably want.
* Release build instead of a debug build.
* Config for what the server address is so the clients connect to the right one.
* Some deployment stuff to deploy the server to digitalocean.

There's gonna be another script too called `setup_server_host.sh` that is a script you can run on a freshly provisioned digitalocean ubuntu vm that sets up any of the dependencies you need to run the server. This is mostly gonna just be like the upstart stuff and a caddyfile. Then travis can push the build artifacts to the server and restart it (a couple of scp/ssh commands) I'll need to make sure creds and everything line up everywhere.
