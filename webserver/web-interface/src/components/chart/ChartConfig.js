export default function getShotChartConfig(theme) {
  return {
    animation: false,
    responsive: true,
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
      y1: {
        type: 'linear',
        display: true,
        position: 'left',
        suggestedMin: 0,
        suggestedMax: 100,
        ticks: {
          fontColor: '#fff',
        },
      },
      y2: {
        type: 'linear',
        display: true,
        position: 'right',
        suggestedMin: 0,
        suggestedMax: 12,
        ticks: {
          fontColor: theme.palette.text.primary,
        },
      },
    },
  };
}
