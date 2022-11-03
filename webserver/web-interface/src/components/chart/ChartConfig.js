export default function getShotChartConfig(theme) {
  return {
    animation: false,
    responsive: true,
    maintainAspectRatio: false,
    interaction: {
      mode: 'index',
      intersect: false,
    },
    stacked: false,
    plugins: {
      legend: {
        display: true,
        position: 'bottom',
      },
    },
    datasets: {
      line: {
        pointRadius: 0,
      },
    },
    legend: {
      labels: {
        fontColor: theme.palette.text.primary,
      },
    },
    scales: {
      x: {
        ticks: {
          color: '#6a6464',
        },
        grid: {
          color: '#6a6464',
        },
      },
      y1: {
        type: 'linear',
        display: true,
        position: 'left',
        suggestedMin: 0,
        suggestedMax: 160,
        grid: {
          color: '#ff5c33',
        },
        ticks: {
          color: '#ff5c33',
        },
      },
      y2: {
        type: 'linear',
        display: true,
        position: 'right',
        suggestedMin: 0,
        suggestedMax: 16,
        grid: {
          color: '#337AB7',
        },
        ticks: {
          color: '#337AB7',
        },
      },
    },
  };
}
