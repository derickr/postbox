// Fetch saved symbol from local storage (using standard localStorage webAPI)
var symbol = window.localStorage.getItem("symbol");

// We use the fake "PBL" symbol as default
if (!symbol) {
  symbol = "PBL";
}

var last_vib_ref = last_send_ref = false;
var ref;
var desc;


function doSendRef() {
	if (last_send_ref != ref) {
		console.log("Sending (as it is different from before): " + ref + ", " + desc);
		Pebble.sendAppMessage({"ref": ref, "desc": desc}, function() {
			last_send_ref = ref;
		} );
	} else {
		console.log("NOT Sending (as it is the same as before): " + ref + ", " + desc);
	}
}

// Fetch stock data for a given stock symbol (NYSE or NASDAQ only) from markitondemand.com
// & send the stock price back to the watch via app message
// API documentation at http://dev.markitondemand.com/#doc
function fetchStockQuote(lat, lon)
{
	var response;
	var req = new XMLHttpRequest();

	// build the GET request
	req.open('GET', "http://maps.derickrethans.nl/maps-postbox/fetch-poi.php?simple=1" + "&lat=" + lat + "&lon=" + lon, true);
	req.onload = function(e) {
		if (req.readyState == 4) {
			// 200 - HTTP OK
			if(req.status == 200) {
				console.log(req.responseText);
				response = JSON.parse(req.responseText);

				ref = "???"; desc = "???";
				if (response.ref && response.desc) {
					ref = response.ref.toString();
					desc = response.desc.toString();
					if (response.score > 99) { 
						desc = "* " + desc;
					} else if (response.score > 49) { 
						desc = "+ " + desc;
					}
				}

				if (response.distance) {
					distance = response.distance.toString();
					wd = '?';
					if (response.w) {
						wd = response.w.toString();
					}
					console.log("Sending: " + distance);
					if (response.distance <= 75 && response.score < 50) {
						if (last_vib_ref != ref) {
							console.log("Instruct to vibrate for " + ref);
							Pebble.sendAppMessage({"distance": distance + " m - " + wd, "vib": true}, function() { doSendRef(); } );
							last_vib_ref = ref;
						} else {
							console.log("Already vibrated for " + ref);
							Pebble.sendAppMessage({"distance": distance + " m - " + wd}, function() { doSendRef(); } );
						}
					} else {
						console.log("Too far for " + ref + ", or already photographed");
						Pebble.sendAppMessage({"distance": distance + " m - " + wd}, function() { doSendRef(); } );
					}
				}

			} else {
				console.log("Request returned error code " + req.status.toString());
			}
		}
	}
	req.send(null);
}

function scrollMap(position)
{
	console.log("Got location...");
	console.log(position.coords.latitude + ', ' + position.coords.longitude);
/*
	Pebble.sendAppMessage({
		"lat": position.coords.latitude.toString(),
		"lon": position.coords.longitude.toString()
	});
*/
	fetchStockQuote(position.coords.latitude.toString(), position.coords.longitude.toString());
}

function handleError(error)
{
}

function doGetLocation()
{
	console.log("Getting location...");
	options = { enableHighAccuracy: true, maximumAge: 10000 };
	navigator.geolocation.getCurrentPosition(scrollMap, handleError, options );

	window.setTimeout(doGetLocation, 15000);
}

window.setTimeout(doGetLocation, 2500);

// Set callback for the app ready event
Pebble.addEventListener(
	"ready",
	function(e) {
		console.log("connect!" + e.ready);
		console.log(e.type);
	}
);

// Set callback for appmessage events
Pebble.addEventListener(
	"appmessage",
	function(e) {
		if (e.payload.ref) {
			var response;
			var req = new XMLHttpRequest();
			var symbol = e.payload.ref;
			var params = "ref=" + symbol;

			req.open('POST', "http://maps.derickrethans.nl/maps-postbox/record-visit.php", true);

			req.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
			req.setRequestHeader("Content-length", params.length);
			req.setRequestHeader("Connection", "close");

			req.onreadystatechange = function() {
				if (req.readyState == 4 && req.status == 200) {
					Pebble.showSimpleNotificationOnPebble(symbol, "Postbox " + symbol + " has been marked as visited!");
				}
			}
			req.send(params);

			console.log("Recorded visit for postbox " + ref);
		}
	}
);
