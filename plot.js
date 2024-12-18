let sheetName = 'Anole';
let sheetId = '1xvuWuXq-Sv5DiOpLmMu9FD9NHat3dILGr39eNHIXJsU';
let chartRanges = ['A1:A20000,B1:G20000', 'A1:A20000,L1:M20000', 'A1:A20000,H1:J20000'];
let maxParticleCountShown = 1000;

// Load the Visualization API and the corechart package
google.charts.load('current', { packages: ['corechart', 'controls', 'line'] });

// Set callback to run when the API is loaded
google.charts.setOnLoadCallback(function() { drawChart(sheetName, 0); });
google.charts.setOnLoadCallback(function() { drawChart(sheetName, 1); });
google.charts.setOnLoadCallback(function() { drawChart(sheetName, 2); });

function drawChart(sheetName, chart_index) {
  // URL to fetch Google Sheets data in CSV format (update with your sheet ID and range)

  let chartUrl = 'https://docs.google.com/spreadsheets/d/' + sheetId + '/gviz/tq?sheet=' + sheetName + '&range=' + chartRanges[chart_index];
  console.log(chartUrl);
  const query = new google.visualization.Query(chartUrl);

  query.send(function(response){ handleQueryResponse(response, chart_index) });
}

function handleQueryResponse(response, chart_index) {
  if (response.isError()) {
    console.error('Error fetching data: ' + response.getMessage());
    return;
  }

  const data = response.getDataTable();

  // Convert the first column (Datetime) to JavaScript Date objects
  for (let i = 0; i < data.getNumberOfRows(); i++) {
    data.setValue(i, 0, new Date(data.getValue(i, 0)));
  }

  // Create a dashboard
  const dashboard = new google.visualization.Dashboard(
    document.getElementById('dashboard_div' + chart_index.toString())
  );

  // get the current time and prev time in milliseconds
  const now = new Date();
  const oneDayAgo = new Date(now.getTime() - 24 * 60 * 60 * 1000);

  // Create a date range filter
  const dateRangeFilter = new google.visualization.ControlWrapper({
    controlType: 'ChartRangeFilter',
    containerId: 'filter_div' + chart_index.toString(),
    state: {
      range: {
        start: oneDayAgo,
        end: now
      }
    },
    options: {
      filterColumnIndex: 0,
      ui: {
        chartType: 'LineChart',
        chartOptions: {
          height: 50,
          hAxis: { format: 'MM/dd/yyyy HH:mm',
            'baselineColor': 'none'},
          chartArea: { 'width': '70%', 'height': '70%' },
        },
        minRangeSize: 0.5*86400000 // 12 hours in milliseconds
      }
    }
  });

  let curMaxParticleCountShown = maxParticleCountShown;
  let title = 'Counts (per 0.1L air)';
  if (chart_index === 2) {
    curMaxParticleCountShown = 20;
    title = 'Counts (μg/m³)';
  }
  let vAxes = {
    0:
      {
        title: title,
        format: 'decimal',
        viewWindow: {
          min: 0,
          max: curMaxParticleCountShown,
        }
      }
    };
  let series = {};
  if (chart_index === 1) {
    vAxes = {
      0: {
          title: 'Temperature (°F)',
          format: 'decimal',
          viewWindow: {
            min: 65,
            max: 80,
          },
          textStyle: {color: '#dc3c14'},
        },
      1: {
          title: 'Humidity (%)',
          format: 'decimal',
          viewWindow: {
            min: 35,
            max: 65,
          },
          textStyle: { color: '#3464cc'}
        }
    };
    series = {
        1: { targetAxisIndex: 0 },  // Temperature on the second vertical axis
        0: { targetAxisIndex: 1 }, // Humidity on the first vertical axis
      };
  } else {
  }

  // Create a line chart for temperature and humidity
  const lineChart = new google.visualization.ChartWrapper({
    chartType: 'LineChart',
    containerId: 'chart_div' + chart_index.toString(),
    options: {
      title: '',
      curveType: 'function',
      legend: { position: 'bottom' },
      hAxis: {
        title: '',
        format: 'MM/dd/yyyy HH:mm',
        gridlines: { count: -1 }
      },
      vAxes: vAxes,
      series: series
    }
  });

  // Bind the date range filter and the line chart to the dashboard
  dashboard.bind(dateRangeFilter, lineChart);

  // Draw the dashboard with the data
  dashboard.draw(data);
  console.log(data);
}

// JavaScript to select and mark an element as active
function setActiveByDataValue(value) {
    // Remove 'active' class from all elements with 'tablinks' class
    document.querySelectorAll('.tablinks.active').forEach((el) => {
        el.classList.remove('active');
    });

    // Select element by data-name and add 'active' class
    const element = document.querySelector(`[data-name="${value}"]`);
    if (element) {
        element.classList.add('active');
    }
}

document.addEventListener("DOMContentLoaded", function() {
  setActiveByDataValue(sheetName);

  // when button is clicked, load that sheet's data
  document.querySelectorAll('.tablinks').forEach(button => {
    button.addEventListener('click', function() {
      sheetName = this.getAttribute('data-name');
      setActiveByDataValue(sheetName);
      drawChart(sheetName, 0);
      drawChart(sheetName, 1);
      drawChart(sheetName, 2);
    });
  });
});
