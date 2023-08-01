import React from 'react';
import { useTheme } from '@mui/material';
import useSensorStateStore from '../../state/SensorStateStore';
import GaugeChart from '../chart/GaugeChart';

export default function TemperatureGauge({ targetTemperature }: { targetTemperature: number}) {
  const theme = useTheme();
  const temperature = useSensorStateStore((state) => state.sensorState.waterTemperature);

  return (
    <GaugeChart
      value={temperature}
      primaryColor={theme.palette.temperature.main}
      maxValue={targetTemperature}
      flashAfterValue={120}
      unit="°C"
    />
  );
}
