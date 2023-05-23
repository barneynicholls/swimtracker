let map;

let maps = [];


function getData() {

    fetch('./swim.json')
        .then((response) => {
            return response.text();
        })
        .then((data) => {
            let rows = data.split('\n');

            let swims = [];
            let swim = [];

            rows.map(function (record) {
                const info = JSON.parse(record);

                if (info.recording == "1" && !isNaN(Date.parse(info.date))) {
                    const rec = {
                        "created": info.date,
                        "temp": info.temp,
                        "lat": info.lat,
                        "lng": info.lng
                    }

                    swim.push(rec);
                }

                if (info.recording == "0" && swim.length > 0) {

                    swims.push(swim);
                    swim = [];
                }
            });

            if (swim.length > 0) {
                swims.push(swim);
            }

            showSwims(swims);
        })
        .catch(function (error) {
            console.log(error);
        });
}

async function showSwims(swims) {
    const swimContainer = document.getElementById("swims");

    const { Map } = await google.maps.importLibrary("maps");

    for (let i = swims.length - 1; i >= 0; i--) {

        const swim = swims[i];

        const card = mapTmpl.content.cloneNode(true);

        const mapContainer = card.querySelector('[data-id="map"]');

        var map = new Map(mapContainer, {
            center: { lat: 50.18, lng: -4.97 },
            zoom: 8,
            mapTypeId: 'hybrid'
        });

        const flightPath = new google.maps.Polyline({
            path: swim,
            geodesic: false,
            strokeColor: "#FF0000",
            strokeOpacity: 1.0,
            strokeWeight: 2,
        });

        flightPath.setMap(map);
        zoomToObject(flightPath, map);

        const stats = getSwimStats(swim);

        const avgTemp = card.querySelector('[data-id="avgTemp"]');
        avgTemp.innerHTML = stats.avgTemp.toString();

        const minTemp = card.querySelector('[data-id="minTemp"]');
        minTemp.innerHTML = stats.minTemp.toString();

        const maxTemp = card.querySelector('[data-id="maxTemp"]');
        maxTemp.innerHTML = stats.maxTemp.toString();

        const rngTemp = card.querySelector('[data-id="rngTemp"]');
        rngTemp.innerHTML = stats.rngTemp.toString();

        const distance = card.querySelector('[data-id="distance"]');
        distance.innerHTML = stats.distance.toString();

        swimContainer.appendChild(card);
    }
}



async function initMap() {
    //@ts-ignore
    getData();
}

function zoomToObject(obj, map) {
    var bounds = new google.maps.LatLngBounds();
    var points = obj.getPath().getArray();
    for (var n = 0; n < points.length; n++) {
        bounds.extend(points[n]);
    }
    map.fitBounds(bounds);
}

function getDistanceFromLatLonInKm(a, b) {
    let lat1 = a.lat;
    let lon1 = a.lng;
    let lat2 = b.lat;
    let lon2 = b.lng;
    var R = 6371; // Radius of the earth in km
    var dLat = deg2rad(lat2 - lat1);  // deg2rad below
    var dLon = deg2rad(lon2 - lon1);
    var a =
        Math.sin(dLat / 2) * Math.sin(dLat / 2) +
        Math.cos(deg2rad(lat1)) * Math.cos(deg2rad(lat2)) *
        Math.sin(dLon / 2) * Math.sin(dLon / 2)
        ;
    var c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a));
    var d = R * c; // Distance in km
    return d;
}

function deg2rad(deg) {
    return deg * (Math.PI / 180)
}

function getSwimStats(swim){
    let sumTemp = 0;
    let minTemp = 1000;
    let maxTemp = -1000;
    let distance = 0;
    let prevLocation = swim[0];

    for (let index = 0; index < swim.length; index++) {

        const element = swim[index];

        distance += getDistanceFromLatLonInKm(prevLocation, element);
        prevLocation = element;

        const temp = element.temp;

        sumTemp += temp;

        minTemp = temp < minTemp ? temp : minTemp;
        maxTemp = temp > maxTemp ? temp : maxTemp;
    }

    distance = distance * 1000;

    distance = distance.toFixed(0);
    minTemp = minTemp.toFixed(1);
    maxTemp = maxTemp.toFixed(1);
    let rngTemp = (maxTemp - minTemp).toFixed(1);

    const avgTemp = (sumTemp / swim.length).toFixed(1);

    return {
        minTemp,
        maxTemp,
        avgTemp,
        rngTemp,
        distance,
    }
}

function addSwim(swim) {
    //     <a href="#" class="list-group-item list-group-item-action d-flex gap-3 py-3" aria-current="true">
    //     <img src="https://maps.googleapis.com/maps/api/staticmap?size=100x100&maptype=hybrid&path=color:0xff0000|weight:5|50.179211,-4.973466|50.179218,-4.973451|50.179222,-4.973434|50.179234,-4.973420|50.179241,-4.973409|50.179249,-4.973394|50.179256,-4.973382&key=AIzaSyBHiKgKb3403Bnil_gtpY1BnmcKSt1c-yU" alt="twbs" width="100" height="100" class="rounded flex-shrink-0">
    //     <div class="d-flex gap-2 w-100 justify-content-between">
    //       <div>
    //         <h6 class="mb-0">List group item heading</h6>
    //         <p class="mb-0 opacity-75">Some placeholder content in a paragraph.</p>
    //       </div>
    //       <small class="opacity-50 text-nowrap">now</small>
    //     </div>
    //   </a>

    //https://maps.googleapis.com/maps/api/geocode/json?latlng=50.179211,-4.973466&key=AIzaSyBHiKgKb3403Bnil_gtpY1BnmcKSt1c-yU
    //https://maps.googleapis.com/maps/api/geocode/json?latlng=50.179211,-4.973466&result_type=locality|political&key=AIzaSyBHiKgKb3403Bnil_gtpY1BnmcKSt1c-yU


    // https://weather.visualcrossing.com/VisualCrossingWebServices/rest/services/timeline/London,UK/2020-12-15T13:00:00?key=YOUR_API_KEY&include=current
    // https://weather.visualcrossing.com/VisualCrossingWebServices/rest/services/timeline/50.179211,-4.973466/2023-05-10T07:41:50?key=84GKVGRSGXY2UFRPE5ANLDL9X&include=current&unitGroup=metric

    // https://api.geoapify.com/v1/geocode/reverse?lat=50.179211&lon=-4.973466&apiKey=cefeadd6610d451db4a07dcf55267a15

    let coords = "";

    let mod = swim.length < 10 ? 1 : 10;

    let sumTemp = 0;
    let minTemp = 1000;
    let maxTemp = -1000;
    let distance = 0;
    let prevLocation = swim[0];

    for (let index = 0; index < swim.length; index++) {

        const element = swim[index];

        distance += getDistanceFromLatLonInKm(prevLocation, element);
        prevLocation = element;

        if (index % mod === 0) {
            coords += '|' + element.lat.toString() + ',' + element.lng.toString();
        }

        var temp = Number(element.temp);

        sumTemp += temp;

        minTemp = temp < minTemp ? temp : minTemp;
        maxTemp = temp > maxTemp ? temp : maxTemp;
    }

    distance = distance * 1000;

    const avgTemp = sumTemp / swim.length;


    const a = document.createElement("a");
    a.setAttribute("href", "#");
    a.setAttribute("class", "list-group-item list-group-item-action d-flex gap-4 py-3");
    a.setAttribute("aria-current", "true");

    const img = document.createElement("img");
    img.src = "https://maps.googleapis.com/maps/api/staticmap?size=100x100&maptype=hybrid&path=color:0xff0000|weight:5" + coords + "&key=" + config.gmaps;
    img.setAttribute("alt", "twbs");
    img.setAttribute("width", "100");
    img.setAttribute("height", "100");
    img.setAttribute("class", "rounded flex-shrink-0");

    a.appendChild(img);

    const content = document.createElement("div");
    content.setAttribute("class", "d-flex gap-2 w-100 justify-content-between");

    const details = document.createElement("div");

    const title = document.createElement("h5");
    title.setAttribute("class", "mb-0");
    title.innerHTML = "Morning Swim";

    details.appendChild(title);

    fetch('https://maps.googleapis.com/maps/api/geocode/json?latlng=50.179211,-4.973466&result_type=locality|political&key=' + config.gmaps)
        .then((response) => {
            return response.json();
        })
        .then((data) => {
            const location = document.createElement("h6");
            location.setAttribute("class", "mb-0");
            location.innerHTML = data.results[0].formatted_address;
            details.appendChild(location);
        })
        .catch(function (error) {
            console.log(error);
        });



    const tempElement = document.createElement("p");
    tempElement.setAttribute("class", "mb-0 opacity-75");
    tempElement.innerHTML = "Avg Temp: " + avgTemp.toFixed(1) + "c Min Temp: " + minTemp.toFixed(1) + "c Max Temp: " + maxTemp.toFixed(1) + "c Range: " + (maxTemp - minTemp).toFixed(1) + "c";

    details.appendChild(tempElement);

    const distanceElement = document.createElement("p");
    distanceElement.setAttribute("class", "mb-0 opacity-75");
    distanceElement.innerHTML = "Distance: " + distance.toFixed(0) + "m";

    details.appendChild(distanceElement);


    content.appendChild(details);

    const created = document.createElement("small");
    created.setAttribute("class", "opacity-50 text-nowrap");
    created.innerHTML = "3 hours ago";

    content.appendChild(created);

    a.append(content);

    document.getElementById("swim-list").appendChild(a);
}

function degToCompass(num) {
    while (num < 0) num += 360;
    while (num >= 360) num -= 360;
    val = Math.round((num - 11.25) / 22.5);
    arr = ["N", "NNE", "NE", "ENE", "E", "ESE", "SE",
        "SSE", "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW"];
    return arr[Math.abs(val)];
}

initMap();
