var http = require('http'),
	util = require('util'),
	ws = require("nodejs-websocket"),
	net = require('net'),
	rconsole = require('rconsole'),
	backgrounder = require('backgrounder');

// Configurable stuff
var	port = 8000, // The port to listen on
	address = undefined, // Interface to listen on, undefined for all
	scrapeInterval = 60000; // How often to collect 'active user' data

// Globals
var clients = [],
	scraper,
	activeUsers = { 
		shades : 0, // Stay pure
		purps : 0, // Filth
		blues : 0, // Unclean but not as bad as the purps
		greens : 0, // Currently bragging a lot
		yellers : 0, // People who didn't want to be green
		oranges : 0, // Thrill seekers
		reds : 0, // Dangerous extremists
		pales : 0 // Innocents
	};

// Send string 'data' to all connected clients
var broadcast = function(data) {
	for(var c in clients)
		clients[c].write(data);
}

// Set up rconsole for logging to syslog, I guess
var initLogs = function() {
	console.set(
		{	'facility' : 'local3',
			'title' : 'button-barfer',
			'syslogHashTags' : false
		}
	);
}

// Cause the scraper background process to do a scrape, apply its findings
var scrape = function() {
	scraper.send(
		{ 'scrape' : true },
		function(message) {
			for(var m in message) {
				if(typeof activeUsers[m] == typeof message[m])
					activeUsers[m] = message[m];
			}
		}
	);
}

// Set up the scraper background process, do a scrape, set the scrape interval
var initScraper = function() {
	scraper = backgrounder.spawn(
		__dirname + "/scraper.js",
		{ },
		function() {
			console.log("Scraper started.");
		}
	);
	scrape();
	setInterval(scrape, scrapeInterval);
}

/*	Get the URL that our websocket client will connect to, set up the client.
	Largely stolen from The Button Monitor (http://jamesrom.github.io/ */
var initHTTP = function() {
	http.get(
		"http://cors-unblocker.herokuapp.com/get?url=https%3A%2F%2Fwww.reddit.com%2Fr%2Fthebutton",
		function(res) {
			var body = "";
			res.on(
				"data",
				function(data) {
					body += data;
				}
			);
			res.on(
				"end",
				function() {
					var regex = /"(wss:\/\/wss\.redditmedia\.com\/thebutton\?h=[^"]*)"/g;
					webSocketURL = regex.exec(body)[1];
					initWebSocket(webSocketURL);
				}
			);
		}
	);
}

// Set up our websocket connection; called by initHTTP()
var initWebSocket = function(webSocketURL) {

	var wsHandle = ws.connect(
		webSocketURL,
		{},
		function() {
			console.log("Connected to " + webSocketURL);
		}
	);

	wsHandle.on(
		"close",
		function(code, reason) {
			console.log(code + ", " + reason);
		}
	);

	// We could maybe do initHTTP again from here
	wsHandle.on(
		"error",
		function(err) {
			console.log(err);
		}
	);

	// If we got text, try to parse it and barf it at the clients
	wsHandle.on(
		"text",
		function(data) {
			try {
				var tmp = JSON.parse(data);
				if(typeof tmp.payload == "undefined")
					return;
				// Clients receive one value per line, in the following order
				broadcast(
					util.format(
						"%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n",
						tmp.payload.participants_text,
						tmp.payload.seconds_left,
						activeUsers.shades,
						activeUsers.purps,
						activeUsers.blues,
						activeUsers.greens,
						activeUsers.yellers,
						activeUsers.oranges,
						activeUsers.reds,
						activeUsers.pales
					)
				);
			} catch(err) {
				// Meh, so there was an error. Who cares?
			}
		}
	);

}

/*	This is the thing that the Arduino connects to. A lot of this is not
	necessary, but I copied this from another project of mine. */
var initServer = function(port, address) {

	var server = net.createServer(

		{ 'allowHalfOpen' : true },

		function(c) {

			// broadcast() needs access to our clients
			clients.push(c);

			console.log(c.remoteAddress + " connected");

			// If a client sends something, we'll log but otherwise ignore it
			clients[clients.length - 1].on(
				'data',
				(function(index) {
					return function(data) {
						console.log(
							"%s: %s",
							clients[index].remoteAddress,
							data.toString().trim()
						);
					}
				})(clients.length - 1)
			);

			clients[clients.length - 1].on(
				'end',
				(function(index) {
					return function() {
						console.log(
							"%s disconnected",
							clients[index].remoteAddress
						);
						clients[index].end();
					}
				})(clients.length - 1)
			);

			clients[clients.length - 1].on(
				'error',
				(function(index) {
					return function(error) {
						console.log(
							"%s: Error: %s",
							clients[index].remoteAddress,
							error
						);
					}
				})(clients.length - 1)
			);

			// Remove a client from the clients array on connection close
			clients[clients.length - 1].on(
				'close',
				(function(index) {
					return function() {
						clients.splice(index, 1);
					}
				})(clients.length - 1)
			);

		}

	);

	server.on(
		"error",
		function(err) {
			console.log("Server error: " + err);
			process.exit();
		}
	);

	// Start listening for client connections
	server.listen(
		port,
		address,
		function() {
			console.log(
				'Listening on %s:%s ...',
				port,
				(typeof address == "undefined") ? "*" : address
			);
		}
	);

}

// Set up the order of things
var main = function() {
	initLogs();
	initServer(port, address);
	initHTTP();
	initScraper();
}

// Do the things
main();