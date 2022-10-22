// update these vars to change chart datapoints
let temp = 0;
let press = 0;
let flow = 0;
let weight = 0;
let counter = 0;
let minutes = 0;
let seconds = 0;

const modeBtn = document.getElementById('mode');
modeBtn.onchange = (e) => {
  if (modeBtn.checked === true) {
    document.documentElement.classList.remove("light")
    document.documentElement.classList.add("dark")
    window.localStorage.setItem('mode', 'dark');
  } else {
    document.documentElement.classList.remove("dark")
    document.documentElement.classList.add("light")
    window.localStorage.setItem('mode', 'light');
  }
}

const mode = window.localStorage.getItem('mode');
if (mode == 'dark') {
  modeBtn.checked = true;
  document.documentElement.classList.remove("light")
  document.documentElement.classList.add("dark")
}

if (mode == 'light') {
  modeBtn.checked = false;
  document.documentElement.classList.remove("dark")
  document.documentElement.classList.add("light")
}

const data = {
    // labels: labels,
    // tension: 0.1,
    borderCapStyle: "round",
    datasets: [
        {
        label: 'Temperature',
        backgroundColor: '#ef4040',
        borderColor: '#ef4040',
        fill: false,
        // stepped: true,
        data: temp,
        },
        {
        label: 'Pressure',
        tension: 0.3,
        backgroundColor: '#6296C5',
        borderColor: '#6296C5',
        fill: false,
        data: press,
        },
        {
        label: 'Flow',
        tension: 0.3,
        backgroundColor: '#844B48',
        borderColor: '#844B48',
        fill: false,
        data: flow,
        },
        {
        label: 'Weight',
        tension: 0.3,
        backgroundColor: '#ffb53e',
        borderColor: '#ffb53e',
        fill: false,
        data: weight,
        }
    ],
};

const config = {
    type: 'line',
    data: data,
    options: {
        animation: false,
        responsive: true,
        maintainAspectRatio: false,
        aspectRatio: 0.6,
        indexAxis: 'x',
        legend: {
            labels: {
               fontColor: '#ffffffc8'
            }
         },
        scales: {
            alignToPixels: true,
            x: {
                ticks: { color: '#337AB7'},
                grid: {
                    color: '#6a6464'
                },
                beginAtZero: true,
            },
            y: {
                ticks: { color: '#337AB7'},
                grid: {
                    color: '#6a6464'
                },
                beginAtZero: true,
                min: 0,
                max: 150,
            },
        },
        elements: {
            point: {
                radius: 0 // default to disabled in all datasets
            }
        },
        spanGaps: true,
        borderJoinStyle: 'round',
        //showLine: false // disable for all datasets
    }
};

const liveSystemDataChart = new Chart(
    document.getElementById('liveSystemDataChart'),
    config
);


function millisToMinutesAndSeconds(sec,min) {
    return (min + ":" + sec);
}

/* Updating the chart datasets, hardcoded for better visibility on what's happening 
during the development process, then needs to be switched to "foreach" based addressing.
Updating via the interval function below this one, it's shit but also gives a known output timing 
which also helps when troubleshooting shit.*/
function updateChart() {
    // const s = new Date().getUTCSeconds();
    // const m = new Date().getUTCMilliseconds();
    // minSecMillis = millisToMinutesAndSeconds(s,m);
    counter++;
    // dataTemperature = 93;
    // dataPressure = Math.floor(Math.random() * 5)+1;
    // dataFlow = Math.floor(Math.random() * 2) + 1;
    // dataWeight += dataFlow; //Math.floor(Math.random() * 40);

    liveSystemDataChart.data.datasets[0].data.push(temp);
    liveSystemDataChart.data.datasets[1].data.push(press*10);
    liveSystemDataChart.data.datasets[2].data.push(flow*10);
    liveSystemDataChart.data.datasets[3].data.push(weight);
    liveSystemDataChart.data.labels.push(millisToMinutesAndSeconds(seconds,minutes));

    // Shift chart data to the left when charts have been populated with 100 elements
    // THere must be a calling function of cahrtJs to check how many elements have populated the chart
    // but nobody got time to find this shit now.
    if (counter >=100) {
        //shifts data at the beginnig  of the chart
        liveSystemDataChart.data.labels.shift();
        liveSystemDataChart.data.datasets[0].data.shift();
        liveSystemDataChart.data.datasets[1].data.shift();
        liveSystemDataChart.data.datasets[2].data.shift();
        liveSystemDataChart.data.datasets[3].data.shift();
    }

    // Nedds to be called otherwise the chart doesn't update
    liveSystemDataChart.update('none');
};
// runs the specified fucntion every 150ms
setInterval(function(){updateChart()}, 150);

// Updates the pie cahrts above the line chart.
function updatePieCharts() {

    $('#easypiechart-temp').data('easyPieChart').update(temp);
    document.getElementById('percentTemp').textContent = temp + "°C";
    $('#easypiechart-press').data('easyPieChart').update(press*10);
    document.getElementById('percentPressure').textContent = press + "bar";
    $('#easypiechart-flow').data('easyPieChart').update(flow*10);
    document.getElementById('percentFlow').textContent = flow + "ml/s";
    $('#easypiechart-weight').data('easyPieChart').update(weight);
    document.getElementById('percentWeight').textContent = weight + "g";

}
//update pie cahrt instances every 150 millis
setInterval(function() {updatePieCharts()}, 150);


//SSE handling

if (!!window.EventSource) {
    var source = new EventSource('/');
  
    source.addEventListener('open', function(e) {
      console.log("Events Connected");
    }, false);
  
    source.addEventListener('error', function(e) {
      if (e.target.readyState != EventSource.OPEN) {
        console.log("Events Disconnected");
      }
    }, false);
  
    source.addEventListener('message', function(e) {
      console.log("message", e.data);
    }, false);
  
    source.addEventListener('temp', function(e) {
      console.log("temp", e.data);
      temp = e.data;
    }, false);
  
    source.addEventListener('press', function(e) {
      console.log("press", e.data);
      press = e.data;
    }, false);

    source.addEventListener('flow', function(e) {
        console.log("flow", e.data);
        flow = e.data;
      }, false);

      source.addEventListener('weight', function(e) {
        console.log("weight", e.data);
        weight = e.data;
      }, false);
  
    source.addEventListener('seconds', function(e) {
      console.log("seconds", e.data);
      seconds = e.data;
    }, false);

    source.addEventListener('minutes', function(e) {
        console.log("minutes", e.data);
        minutes = e.data;
    }, false);
}


  // Upload btn
function dataUpload() {
    var x = document.getElementById("progress-bar");
    if (x.style.display === "none") {
      x.style.display = "block";
    }
  }