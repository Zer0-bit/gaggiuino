import { create } from 'zustand';
import { devtools } from 'zustand/middleware';
import { SensorState } from '../models/models';

interface SensorStateStore {
  sensorState: SensorState,
  updateSensorState: (newState: SensorState) => void,
}

const useSensorStateStore = create<SensorStateStore>()(
  devtools(
    (set) => ({
      sensorState: {
        brewActive: false,
        steamActive: false,
        hotWaterSwitchState: false,
        temperature: 0,
        waterTemperature: 0,
        pressure: 0,
        pumpFlow: 0,
        weightFlow: 0,
        weight: 0,
        waterLevel: 0,
      },
      updateSensorState: (newState: SensorState) => set(() => ({ sensorState: newState })),
    }),
  ),
);

export default useSensorStateStore;
