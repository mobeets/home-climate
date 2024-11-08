let charts = ['https://docs.google.com/spreadsheets/d/1xvuWuXq-Sv5DiOpLmMu9FD9NHat3dILGr39eNHIXJsU/gviz/tq?sheet=Anole&range=A1:A20000,L1:M20000', 'https://docs.google.com/spreadsheets/d/1xvuWuXq-Sv5DiOpLmMu9FD9NHat3dILGr39eNHIXJsU/gviz/tq?sheet=Anole&range=A1:A20000,B1:I20000'];

// Load the Visualization API and the corechart package
google.charts.load('current', { packages: ['corechart', 'controls', 'line'] });

// Set a callback to run when the API is loaded
google.charts.setOnLoadCallback(function() { drawChart(0); });
google.charts.setOnLoadCallback(function() { drawChart(1); });

function drawChart(chart_index) {
  // URL to fetch Google Sheets data in CSV format (update with your sheet ID and range)
  const query = new google.visualization.Query(charts[chart_index]);

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

  // Create a date range filter
  // n.b. would need to set state: {range.start: and range.end}
  // in order initialize to it showing the most recent date
  const dateRangeFilter = new google.visualization.ControlWrapper({
    controlType: 'ChartRangeFilter',
    containerId: 'filter_div' + chart_index.toString(),
    options: {
      filterColumnIndex: 0,
      ui: {
        chartType: 'LineChart',
        chartOptions: {
          height: 50,
          hAxis: { format: 'MM/dd/yyyy HH:mm',
            'baselineColor': 'none'},
          chartArea: { 'width': '70%' },
        },
        minRangeSize: 0.5*86400000 // 12 hours in milliseconds
      }
    }
  });

  let vAxes = {0: {title: 'Counts (per 0.1L air)', format: 'decimal'}};
  let series = {};
  if (chart_index === 0) {
    vAxes = {
      0: { title: 'Temperature (°F)', format: 'decimal', textStyle: {color: '#dc3c14'}},
      1: { title: 'Humidity (%)', format: 'decimal', textStyle: { color: '#3464cc'}}
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
