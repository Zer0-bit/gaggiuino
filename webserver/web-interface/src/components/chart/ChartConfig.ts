import { Theme, alpha } from '@mui/material';
import { ChartOptions } from 'chart.js';

export function getShotChartConfig(theme: Theme, onHover?: (index: number) => void): ChartOptions<'line'> {
  return {
    animation: false,
    responsive: true,
    maintainAspectRatio: false,
    onHover: (event, elements) => {
      if (onHover && elements && elements.length > 0 && elements[0].index) {
        const { index } = elements[0]; // get the index of the hovered element
        onHover(index);
      }
    },
    interaction: {
      mode: 'index',
      intersect: false,
    },
    plugins: {
      legend: {
        display: false,
      },
      tooltip: {
        enabled: false,
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
          callback: (tickValue) => `${tickValue as number / 1000}`,
        },
        grid: {
          color: theme.palette.divider,
        },
        min: 0,
        suggestedMax: 60000,
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

export function getProfilePreviewChartConfig({ theme, onClick, max }: {theme: Theme, onClick: (dataIndex: number) => void, max?: number}): ChartOptions<'line'> {
  return {
    animation: false,
    responsive: true,
    maintainAspectRatio: false,
    onClick: (event, elements) => {
      if (onClick && elements && elements.length > 0 && elements[0].index) {
        const { index } = elements[0]; // get the index of the clicked element
        onClick(index);
      }
    },
    layout: {
      padding: 0,
    },
    interaction: {
      mode: 'index',
      intersect: false,
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
        tension: 0,
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
        max,
      },
      y1: {
        type: 'linear',
        display: false,
        position: 'left',
        min: 0,
        suggestedMax: 105,
        grid: {
          display: false,
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
        max: 15,
        grid: {
          display: false,
        },
        ticks: {
          display: false,
        },
      },
    },
  };
}
