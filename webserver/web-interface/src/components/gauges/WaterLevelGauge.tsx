import React from 'react';
import GaugeLiquid from '../chart/GaugeLiquid';
import useSensorStateStore from '../../state/SensorStateStore';

export default function WaterLevelGauge() {
  const waterLevel = useSensorStateStore((state) => state.sensorState.waterLevel);

  return <GaugeLiquid value={waterLevel} />;
}
