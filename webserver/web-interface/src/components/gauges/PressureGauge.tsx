import React from 'react';
import { useTheme } from '@mui/material';
import useSensorStateStore from '../../state/SensorStateStore';
import GaugeChart from '../chart/GaugeChart';

export default function PressureGauge() {
  const theme = useTheme();
  const pressure = useSensorStateStore((state) => state.sensorState.pressure);

  return (
    <GaugeChart
      value={pressure}
      primaryColor={theme.palette.pressure.main}
      title="Pressure"
      unit="bar"
      maxValue={14}
    />
  );
}
