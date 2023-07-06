import React from 'react';
import { Doughnut } from 'react-chartjs-2';
import {
  Chart as ChartJS,
  ArcElement,
  Title,
  ChartOptions,
  Color,
  ChartData,
} from 'chart.js';
import { useTheme } from '@mui/material';
import GaugeCentralTextPlugin from './GaugeCentralTextPlugin';

ChartJS.register(ArcElement, Title, GaugeCentralTextPlugin);

interface GaugeChartProps {
  value: number;
  maxValue?: number;
  primaryColor: Color;
  unit?: string;
  title?: string;
}

function GaugeChart({
  value,
  maxValue,
  primaryColor,
  unit,
  title,
}: GaugeChartProps) {
  const theme = useTheme();

  const options: ChartOptions<'doughnut'> = {
    cutout: '89%',
    responsive: true,
    borderColor: primaryColor,
    maintainAspectRatio: false,
    animation: {
      animateRotate: false,
      animateScale: false,
    },
    plugins: {
      tooltip: {
        enabled: false,
      },
      center: {
        text: value.toFixed(1) + unit,
        color: primaryColor,
        maxFontSize: 55,
      },
      title: {
        display: !!title && title.length > 0,
        text: title,
        color: theme.palette.text.secondary,
        font: {
          family: theme.typography.fontFamily,
          weight: 'normal',
        },
      },
    },
  };

  const data: ChartData<'doughnut'> = {
    datasets: [{
      data: [value, Math.max(0, (maxValue || 100) - value)],
      borderWidth: 0,
      backgroundColor: [
        primaryColor,
        theme.palette.divider,
      ],
    }],
  };
  return <Doughnut data={data} options={options} />;
}

export default GaugeChart;

GaugeChart.defaultProps = {
  maxValue: 100,
  unit: '',
  title: '',
};
