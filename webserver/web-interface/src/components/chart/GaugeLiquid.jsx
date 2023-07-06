import React, { useEffect, useRef, useState } from 'react';
import { debounce, useTheme } from '@mui/material';
import GaugeLiquid from 'react-liquid-gauge';
import AspectRatioBox from '../layout/AspectRatioBox';
import { GaugeTitle } from './GaugeChart';

function GaugeLiquidComponent({ value = 0 }) {
  const theme = useTheme();
  const gaugeRef = useRef(null);
  const [gaugeSize, setGaugeSize] = useState({ width: 0, height: 0 });

  const calculateGaugeSize = debounce(() => {
    if (gaugeRef.current) {
      const { width, height } = gaugeRef.current.getBoundingClientRect();
      setGaugeSize({ width, height });
    }
  }, 300);

  useEffect(() => {
    calculateGaugeSize();
    window.addEventListener('resize', calculateGaugeSize);

    return () => {
      window.removeEventListener('resize', calculateGaugeSize);
    };
  }, [gaugeRef, calculateGaugeSize]);

  return (
    <>
      <GaugeTitle sx={{ mb: theme.spacing(1) }}>Water Level</GaugeTitle>
      <AspectRatioBox ref={gaugeRef}>
        {gaugeSize.width > 10 && (
        <GaugeLiquid
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
            fill: theme.palette.water.main,
            fontFamily: theme.typography.fontFamily,
          }}
        />
        )}
      </AspectRatioBox>
    </>
  );
}

export default GaugeLiquidComponent;
