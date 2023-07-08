import { create } from 'zustand';
import { devtools, persist } from 'zustand/middleware';
import { Shot, ShotSnapshot } from '../models/models';

interface ShotDataStore {
  latestShotDatapoint: ShotSnapshot,
  currentShot: Shot,
  shotHistory: Shot[],
  startNewShot: () => void,
  addShotDatapoint: (shotDatapoint: ShotSnapshot) => void,
  addShotToHistory: (shot: Shot) => void,
}

export function isShotLongEnoughToBeStored(shot: Shot): boolean {
  return shot.datapoints.length > 0 && shot.datapoints[shot.datapoints.length - 1].timeInShot > 8000;
}

export function isNewShotStarted(currentShot: Shot, newShotDatapoint: ShotSnapshot):boolean {
  if (currentShot.datapoints.length === 0) return true;

  const latestShotDatapoint = currentShot.datapoints[currentShot.datapoints.length - 1];
  return latestShotDatapoint && latestShotDatapoint.timeInShot > newShotDatapoint.timeInShot;
}

const useShotDataStore = create<ShotDataStore>()(
  devtools(
    persist(
      (set, get) => ({
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
        currentShot: { time: 0, datapoints: [] },
        shotHistory: [],

        startNewShot: () => set(() => ({ currentShot: { time: Date.now(), datapoints: [] } })),

        addShotToHistory: (shot:Shot) => set((state) => {
          if (isShotLongEnoughToBeStored(shot)) {
            const updatedShotHistory = [...state.shotHistory, shot];
            while (updatedShotHistory.length > 10) {
              updatedShotHistory.shift(); // This will remove the first element from the array
            }
            return { shotHistory: updatedShotHistory };
          }
          return {};
        }),

        addShotDatapoint: (shotDatapoint: ShotSnapshot) => {
          if (isNewShotStarted(get().currentShot, shotDatapoint)) {
            get().addShotToHistory(get().currentShot);
            get().startNewShot();
          }
          const { currentShot } = get();
          const shot = { ...(currentShot) };
          shot.datapoints.push(shotDatapoint);
          set(() => ({ currentShot: shot, latestShotDatapoint: shotDatapoint }));
        },
      }),
      {
        name: 'shot-storage',
        partialize: (state) => ({ shotHistory: state.shotHistory }),
      },
    ),
  ),
);

export default useShotDataStore;
