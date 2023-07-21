import React, {
  useEffect, useMemo, useRef, useState,
} from 'react';
import { Doughnut } from 'react-chartjs-2';
import {
  Chart as ChartJS,
  ArcElement,
  Title,
  ChartOptions,
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
  primaryColor: string;
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
  const theme = useTheme();
  const [shouldFlash, setShouldFlash] = useState(false);
  // eslint-disable-next-line @typescript-eslint/no-explicit-any
  const chartRef = useRef<ChartJS<'doughnut'>>();

  useEffect(() => {
    const newShouldFlash = flashAfterValue !== undefined && value > flashAfterValue;
    setShouldFlash(newShouldFlash);
  }, [value, flashAfterValue]);

  const options: ChartOptions<'doughnut'> = useMemo(() => ({
    cutout: '89%',
    responsive: true,
    maintainAspectRatio: true,
    animation: {
      animateScale: false,
      animateRotate: false,
    },
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
    borderColor: primaryColor,
    backgroundColor: (ctx) => (ctx.dataIndex === 0 ? primaryColor : theme.palette.divider),
    animations: {
      backgroundColor: shouldFlash ? {
        duration: 500,
        easing: 'linear',
        type: 'color',
        from: (ctx) => (ctx.dataIndex === 0 ? primaryColor : theme.palette.divider),
        to: (ctx) => (ctx.dataIndex === 0 ? lighten(primaryColor, 0.5) : theme.palette.divider),
        loop: true,
      } : false,
    },

  }), [primaryColor, unit, shouldFlash, theme]);

  // update data without re-render of the whole component
  useEffect(() => {
    if (chartRef.current?.data.datasets[0]) {
      chartRef.current.data.datasets[0].data = [value, Math.max(0, maxValue - value)];
      chartRef.current.update();
    }
  }, [value, maxValue]);

  const data: ChartData<'doughnut'> = useMemo(() => ({
    datasets: [{
      data: [0, 0],
    }],
  }), []);

  return (
    <>
      {title && <GaugeTitle>{title}</GaugeTitle>}
      <AspectRatioBox><Doughnut ref={chartRef} data={data} options={options} /></AspectRatioBox>
    </>
  );
}

export default GaugeChart;
