import { Theme, alpha } from '@mui/material';
import { ChartOptions } from 'chart.js';

export function getShotChartConfig(theme: Theme): ChartOptions<'line'> {
  return {
    animation: false,
    responsive: true,
    maintainAspectRatio: false,
    interaction: {
      mode: 'index',
      intersect: false,
    },
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
        axis: 'x',
        type: 'linear',
        ticks: {
          color: theme.palette.text.secondary,
        },
        grid: {
          color: theme.palette.divider,
        },
        min: 0,
        suggestedMax: 60,
      },
      y1: {
        type: 'linear',
        display: true,
        position: 'left',
        min: 0,
        suggestedMax: 100,
        grid: {
          color: alpha(theme.palette.temperature.main, 0.5),
          tickBorderDash: [3, 3],
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
          tickBorderDash: [3, 3],
        },
        ticks: {
          color: theme.palette.pressure.main,
        },
      },
    },
  };
}

export function getProfilePreviewChartConfig(theme: Theme): ChartOptions<'line'> {
  return {
    animation: false,
    responsive: true,
    maintainAspectRatio: false,
    layout: {
      padding: 0,
    },
    plugins: {
      tooltip: {
        enabled: false,
      },
      legend: {
        display: false,
      },
    },
    datasets: {
      line: {
        pointRadius: 0,
        borderWidth: 1.5,
        borderDash: [3, 3],
      },
    },
    scales: {
      x: {
        axis: 'x',
        type: 'linear',
        ticks: {
          display: false,
        },
        grid: {
          display: false,
          color: theme.palette.divider,
        },
        min: 0,
      },
      y1: {
        type: 'linear',
        display: false,
        position: 'left',
        min: 0,
        suggestedMax: 100,
        grid: {
          display: false,
          color: alpha(theme.palette.temperature.main, 0.5),
          tickBorderDash: [3, 3],
        },
        ticks: {
          display: false,
        },
      },
      y2: {
        type: 'linear',
        display: false,
        position: 'right',
        min: -0.5,
        max: 12,
        grid: {
          display: false,
          color: alpha(theme.palette.pressure.main, 0.5),
          tickBorderDash: [3, 3],
        },
        ticks: {
          display: false,
        },
      },
    },
  };
}
