import React, { useMemo } from 'react';
import { Doughnut } from 'react-chartjs-2';
import {
  Chart as ChartJS,
  ArcElement,
  Title,
  ChartOptions,
  Color,
  ChartData,
} from 'chart.js';
import {
  Typography, TypographyProps, lighten, useTheme,
} from '@mui/material';
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
  flashAfterValue?: number,
}

export function GaugeChart({
  value,
  maxValue = 100,
  primaryColor,
  unit = '',
  title = '',
  flashAfterValue = undefined,
}: GaugeChartProps) {
  const options: ChartOptions<'doughnut'> = useMemo(() => ({
    cutout: '89%',
    responsive: true,
    maintainAspectRatio: true,
    animation: true,
    borderWidth: 0,
    plugins: {
      tooltip: {
        enabled: false,
      },
      center: {
        unit,
        decimals: 1,
        color: primaryColor,
        maxFontSize: 55,
        minFontSize: 1,
        sidePadding: 10,
      },
    },
  }), [primaryColor, unit]);

  const data: ChartData<'doughnut'> = useMemo(() => ({
    datasets: [{
      data: [value],
      borderColor: primaryColor,
      backgroundColor: primaryColor,
      circumference: (value / maxValue * 360),
      animations: {
        backgroundColor: flashAfterValue && value > flashAfterValue ? {
          duration: 1000,
          easing: 'linear',
          type: 'color',
          from: primaryColor,
          to: lighten(primaryColor as string, 0.5),
          loop: true,
        } : {},
      },
    }],
  }), [value, primaryColor, maxValue, flashAfterValue]);

  return (
    <>
      {title && <GaugeTitle>{title}</GaugeTitle>}
      <AspectRatioBox><Doughnut data={data} options={options} /></AspectRatioBox>
    </>
  );
}

export default GaugeChart;
