/*	The parsing process seems a bit slow, so we'll run this in the background.
	This script is spawned and then called at intervals by the parent script
	(which is index.js).  You don't need to run it directly. */

// Use jsdom 3.1.2, the last node-compatible version, I believe.
var jsdom = require('jsdom');

/*	Have jsdom load /r/thebutton, then use jQuery to parse 'active user' data
	from the sidebar where a <p> element has a <span> of a relevant class. */
var scrape = function(callback) {

	var activeUsers = {}; // We'll jelly some stuff in here later.

	/*	I don't really know what cors-unblocker.herokuapp.com is for, but
		it's used at http://jamesrom.github.io/ and The Button Monitor is
		awesome, so I'll copy. */
	jsdom.env(
		{	'url' : "http://cors-unblocker.herokuapp.com/get?url=https%3A%2F%2Fwww.reddit.com%2Fr%2Fthebutton",
			'scripts' : ["http://code.jquery.com/jquery.js"],
			'done' : function(errors, window) {
				var $ = window.$;
				var types = 0;
				$(".side p").each(
					function() {
						var re = /\.\~(\d*)\sactive\susers/g;
						if($(this).has('span.flair-no-press').length != 0) 
							activeUsers.shades = Number(re.exec($(this).text())[1]);
						 else if($(this).has('span.flair-press-6').length != 0) 
							activeUsers.purps = Number(re.exec($(this).text())[1]);
						 else if($(this).has('span.flair-press-5').length != 0) 
							activeUsers.blues = Number(re.exec($(this).text())[1]);
						 else if($(this).has('span.flair-press-4').length != 0) 
							activeUsers.greens = Number(re.exec($(this).text())[1]);
						 else if($(this).has('span.flair-press-3').length != 0) 
							activeUsers.yellers = Number(re.exec($(this).text())[1]);
						 else if($(this).has('span.flair-press-2').length != 0) 
							activeUsers.oranges = Number(re.exec($(this).text())[1]);
						 else if($(this).has('span.flair-press-1').length != 0) 
							activeUsers.reds = Number(re.exec($(this).text())[1]);
						 else if($(this).has('span.flair-cant-press').length != 0) 
							activeUsers.pales = Number(re.exec($(this).text())[1]);
					}
				);
				window.close(); // Fix annoying memory leak?
				// Send the data we collected along to the callback
				callback(activeUsers);
			}
		}
	);	

}

// Backgrounder junk
process.on(
	'config',
	function(message, callback) {
		console.log("Scraper worker configured");
		callback();
	}
);

// Assume that if the parent sends us a message, it wants us to scrape.
process.on(
	'message',
	function(message, callback) {
		// Scrape and send collected data back to the parent's callback
		scrape(callback);
	}
);

// Backgrounder junk
process.on(
	'terminate',
	function(message, callback) {
		console.log("Scraper worker terminated");
	}
);