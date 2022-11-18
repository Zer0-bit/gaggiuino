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
        labels: {
          color: theme.palette.text.secondary,
        },
      },
    },
    datasets: {
      line: {
        pointRadius: 0,
      },
    },
    scales: {
      x: {
        ticks: {
          color: theme.palette.text.secondary,
        },
        grid: {
          color: theme.palette.divider,
        },
      },
      y1: {
        type: 'linear',
        display: true,
        position: 'left',
        suggestedMin: 0,
        suggestedMax: 160,
        grid: {
          color: theme.palette.temperature.main,
        },
        ticks: {
          color: theme.palette.temperature.main,
        },
      },
      y2: {
        type: 'linear',
        display: true,
        position: 'right',
        suggestedMin: 0,
        suggestedMax: 16,
        grid: {
          color: theme.palette.pressure.main,
        },
        ticks: {
          color: theme.palette.pressure.main,
        },
      },
    },
  };
}
