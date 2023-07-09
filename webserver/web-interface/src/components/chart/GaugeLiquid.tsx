import React, {
  useCallback,
  useEffect, useRef, useState,
} from 'react';
import {
  darken, debounce, lighten, useTheme,
} from '@mui/material';
import LiquidFillGauge from 'react-liquid-gauge';
import AspectRatioBox from '../layout/AspectRatioBox';
import { GaugeTitle } from './GaugeChart';

export default function GaugeLiquidComponent({ value }: { value: number}) {
  const theme = useTheme();
  const gaugeRef = useRef<HTMLElement | null>(null);
  const [gaugeSize, setGaugeSize] = useState({ width: 0, height: 0 });

  const calculateGaugeSize = useCallback(
    () => {
      const debouncedCalculateSize = debounce(() => {
        if (gaugeRef.current) {
          const { width, height } = gaugeRef.current.getBoundingClientRect();
          setGaugeSize({ width, height });
        }
      }, 300);
      debouncedCalculateSize();
    },
    [gaugeRef, setGaugeSize],
  );

  useEffect(() => {
    calculateGaugeSize();
    window.addEventListener('resize', calculateGaugeSize);

    return () => {
      window.removeEventListener('resize', calculateGaugeSize);
    };
  }, [gaugeRef, calculateGaugeSize]);

  return (
    <>
      <GaugeTitle sx={{ mb: theme.spacing(1) }}>Water</GaugeTitle>
      <AspectRatioBox ref={gaugeRef}>
        {gaugeSize.width > 10 && (
        <LiquidFillGauge
          style={{ margin: '0 auto' }}
          width={gaugeSize.width - 10}
          height={gaugeSize.width - 10}
          value={value}
          percent="%"
          textSize={1.3}
          textOffsetX={0}
          textOffsetY={0}
          riseAnimation
          waveAnimation
          waveFrequency={1}
          waveAmplitude={3}
          textStyle={{
            fill: theme.palette.water.main,
            fontFamily: theme.typography.fontFamily,
          }}
          waveTextStyle={{
            fill: theme.palette.mode === 'dark' ? darken(theme.palette.water.main, 0.4) : lighten(theme.palette.water.main, 0.4),
            fontFamily: theme.typography.fontFamily,
          }}
        />
        )}
      </AspectRatioBox>
    </>
  );
}
