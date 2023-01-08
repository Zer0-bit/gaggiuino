import { alpha } from '@mui/material';

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
        min: 0,
        suggestedMax: 100,
        grid: {
          color: alpha(theme.palette.temperature.main, 0.5),
          borderDash: [3, 3],
        },
        ticks: {
          color: theme.palette.temperature.main,
        },
      },
      y2: {
        type: 'linear',
        display: true,
        position: 'right',
        min: 0,
        suggestedMax: 16,
        grid: {
          color: alpha(theme.palette.pressure.main, 0.5),
          borderDash: [3, 3],
        },
        ticks: {
          color: theme.palette.pressure.main,
        },
      },
    },
  };
}
