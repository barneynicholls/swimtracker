document.getElementById('txtFileUpload').addEventListener('change', upload, false);

function upload(evt) {
    var data = null;
    var file = evt.target.files[0];
    var reader = new FileReader();
    reader.readAsText(file);
    reader.onload = function (event) {
        var csvData = event.target.result;

        var parsedCsv = Papa.parse(csvData, {
            header: false
        });

        let swims = [];

        let swim = [];

        let i = 0;
        while (i < parsedCsv.data.length) {
            let row = parsedCsv.data[i];
            if (row[9] == "1" && !isNaN(Date.parse(row[0]))) {

                let entry = createRecord(row);
                swim.push(entry);
            }

            if (row[9] == "0" && swim.length > 0) {
                swims.push(swim);
                swim = [];
            }
            i++;
        }

        console.log(swims);

        let test = swims[0];

        const flightPath = new google.maps.Polyline({
            path: test,
            geodesic: false,
            strokeColor: "#FF0000",
            strokeOpacity: 1.0,
            strokeWeight: 2,
        });

        flightPath.setMap(map);
        zoomToObject(flightPath);

        addSwim(test);

    };
    reader.onerror = function () {
        alert('Unable to read ' + file.fileName);
    };
}

function createRecord(row) {
    return {
        "created": row[0],
        "temperature": Number(row[8]),
        "lat": Number(row[1]),
        "lng": Number(row[2])

    };
}

let map;

async function initMap() {
    //@ts-ignore
    const { Map } = await google.maps.importLibrary("maps");

    map = new Map(document.getElementById("map"), {
        center: { lat: 50.18, lng: -4.97 },
        zoom: 8,
        mapTypeId: 'hybrid'
    });
}

initMap();

function zoomToObject(obj) {
    var bounds = new google.maps.LatLngBounds();
    var points = obj.getPath().getArray();
    for (var n = 0; n < points.length; n++) {
        bounds.extend(points[n]);
    }
    map.fitBounds(bounds);
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

    // https://www.bluetoad.net/external/swim.txt

    const a = document.createElement("a");
    a.setAttribute("href", "#");
    a.setAttribute("class", "list-group-item list-group-item-action d-flex gap-4 py-3");
    a.setAttribute("aria-current", "true");

    const img = document.createElement("img");
    img.src = "https://maps.googleapis.com/maps/api/staticmap?size=100x100&maptype=hybrid&path=color:0xff0000|weight:5|50.179211,-4.973466|50.179218,-4.973451|50.179222,-4.973434|50.179234,-4.973420|50.179241,-4.973409|50.179249,-4.973394|50.179256,-4.973382&key=AIzaSyBHiKgKb3403Bnil_gtpY1BnmcKSt1c-yU";
    img.setAttribute("alt", "twbs");
    img.setAttribute("width", "100");
    img.setAttribute("height", "100");
    img.setAttribute("class", "rounded flex-shrink-0");

    a.appendChild(img);

    const content = document.createElement("div");
    content.setAttribute("class", "d-flex gap-2 w-100 justify-content-between");

    const details = document.createElement("div");

    const title = document.createElement("h6");
    title.setAttribute("class","mb-0");
    title.innerHTML = "Morning Swim";
    
    details.appendChild(title);
    
    const info = document.createElement("p");
    info.setAttribute("class","mb-0 opacity-75");
    info.innerHTML = "Length: 100m Avg Temp: 11.3c";

    details.appendChild(info);

    
    content.appendChild(details);
    
    const created = document.createElement("small");
    created.setAttribute("class","opacity-50 text-nowrap");
    created.innerHTML = "3 hours ago";
    
    content.appendChild(created);

    a.append(content);

    document.getElementById("swim-list").appendChild(a);
}