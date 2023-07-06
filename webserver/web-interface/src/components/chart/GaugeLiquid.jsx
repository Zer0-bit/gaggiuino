import { Typography, useTheme } from '@mui/material';
import React from 'react';
import GaugeLiquid from 'react-liquid-gauge';

function GaugeLiquidComponent({ value = 0, radius = 50 }) {
  const theme = useTheme();
  return (
    <div>
      <div>
        <Typography sx={{
          mb: theme.spacing(1), textAlign: 'center', fontSize: '13px', color: theme.palette.text.secondary,
        }}
        >
          Water Level
        </Typography>
      </div>
      <GaugeLiquid
        style={{ margin: '0 auto' }}
        width={radius / 3.7}
        height={radius / 3.7}
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
    </div>
  );
}

export default GaugeLiquidComponent;
