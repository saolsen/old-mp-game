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
* `sudo apt-get install build-essential libsdl2-dev libsdl2-net-dev libglew-dev -y`

### OSX
On osx everything needed is packaged in this repo.

### Windows
Still tbd, no windows support yet.

## Local Game Development
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

I want to make sure I can easily set up digitalocean and get travis to deploy to it.
I want travis to copy the app bundles and the server, then restart caddy and the server.
I think I just have to set up a user, set up ssh for that user, then just ssh in from travis, gotta just give it the private key.

Set up some s3 creds on the box so we can pull and push to s3. travis pushes builds there and we pull them. then just a script to log into a box, pull the latest release and restart everything.

If I could get travis to create a github release if it's a tag and then push the assets there that would be pretty dope.

Make a few thing things to upload to releases.
for osx
- upload a zip file of the osx client
for linux
- upload a tar.gz file of the linux client
- upload a tar.gz file of the server
- upload a tar.gz file of the website
- TODO: upload the windows client

Then my deploy script can simply log into the box, pull the release down, unpack it to the right spots and restart stuff. Easy.

## Site
The site is hosted with Caddy and built using hugo. To develop locally you probably want both of those installed.

For the site this is what I want.
A nice front page. Says some stuff about the game and the current version.
Download links to the clients.

Maybe this is a better way to do it.
- have it set up a user,
- give that user permission to restart those two service,
- have travis scp the files over and restart the services?

means the osx one will show up a little later, I guess that's ok... nah, I don't like it.
EH, lets try it. we do know right now that osx runs after linux, that's sorta useful.
