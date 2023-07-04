import { create } from 'zustand';
import { devtools } from 'zustand/middleware';
import { ShotSnapshot } from '../models/models';

interface ShotDataStore {
  latestShotDatapoint: ShotSnapshot,
  addShotDatapoint: (shotDatapoint: ShotSnapshot) => void,
}

const useShotDataStore = create<ShotDataStore>()(
  devtools(
    (set) => ({
      latestShotDatapoint: {
        timeInShot: 0,
        pressure: 0,
        pumpFlow: 0,
        weightFlow: 0,
        temperature: 0,
        shotWeight: 0,
        waterPumped: 0,
        targetTemperature: 0,
        targetPumpFlow: 0,
        targetPressure: 0,
      },
      addShotDatapoint: (shotDatapoint: ShotSnapshot) => set(() => ({ latestShotDatapoint: shotDatapoint })),
    }),
  ),
);

export default useShotDataStore;
