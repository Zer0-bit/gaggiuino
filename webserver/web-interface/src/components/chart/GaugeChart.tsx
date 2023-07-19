import React, { useState, useEffect } from 'react';
import { Doughnut } from 'react-chartjs-2';
import {
  Chart as ChartJS,
  ArcElement,
  Title,
  ChartOptions,
  Color,
  ChartData,
} from 'chart.js';
import { Typography, TypographyProps, useTheme } from '@mui/material';
import GaugeCentralTextPlugin from './GaugeCentralTextPlugin';
import AspectRatioBox from '../layout/AspectRatioBox';

ChartJS.register(ArcElement, Title, GaugeCentralTextPlugin);

export function GaugeTitle({ children, sx }: TypographyProps) {
  const theme = useTheme();
  return (
    <Typography
      variant="body2"
      sx={{
        textAlign: 'center',
        color: theme.palette.text.secondary,
        ...sx,
      }}
    >
      {children}
    </Typography>
  );
}

type GaugeChartProps = {
  value: number;
  maxValue?: number;
  primaryColor: Color;
  unit?: string;
  title?: string;
}

export function GaugeChart({
  value,
  maxValue = 100,
  primaryColor,
  unit = '',
  title = '',
}: GaugeChartProps) {
  const theme = useTheme();
  const [setIsFlashing, setIsSteaming] = useState(false);

  useEffect(() => {
    let interval: any;
    if (value > 120) {
      setIsSteaming(value > 120);
      interval = setInterval(() => {
        setIsSteaming((prevIsFlashing) => !prevIsFlashing);
      }, 500); // TO-DO: make the gauge glowing increasing and decreasing intensity instead of flashing
    }

    return () => clearInterval(interval);
  }, [value]);

  const options: ChartOptions<'doughnut'> = {
    cutout: '89%',
    responsive: true,
    maintainAspectRatio: true,
    animation: false,
    borderColor: primaryColor,
    plugins: {
      tooltip: {
        enabled: false,
      },
      center: {
        value: value.toFixed(1),
        unit,
        color: primaryColor,
        maxFontSize: 55,
        minFontSize: 1,
        sidePadding: 10,
      },
    },
  };

  const data: ChartData<'doughnut'> = {
    datasets: [{
      data: [value, Math.max(0, (maxValue || 100) - value)],
      borderWidth: 0,
      backgroundColor: [
        setIsFlashing ? theme.palette.primary.light : primaryColor,
        theme.palette.divider,
      ],
    }],
  };

  return (
    <>
      {title && <GaugeTitle>{title}</GaugeTitle>}
      <AspectRatioBox><Doughnut data={data} options={options} /></AspectRatioBox>
    </>
  );
}

export default GaugeChart;
