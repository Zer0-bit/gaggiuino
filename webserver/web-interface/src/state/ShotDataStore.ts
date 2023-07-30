import { create } from 'zustand';
import { devtools, persist } from 'zustand/middleware';
import { Shot, ShotSnapshot } from '../models/models';

interface ShotDataStore {
  latestShotDatapoint: ShotSnapshot,
  currentShot: Shot,
  shotRunning: boolean,
  shotHistory: Shot[],
  startNewShot: () => void,
  addShotDatapoint: (shotDatapoint: ShotSnapshot) => void,
  addShotToHistory: (shot: Shot) => void,
  removeShotFromHistory: (shot: Shot) => void,
  setShotRunning: (value: boolean) => void,
}

// This constant defines a buffer of time to prevent handling of out-of-order
// socket messages.
const OUT_OF_ORDER_BUFFER_TOLERANCE = 500; // msec
const MAX_SHOT_HISTORY_LENGTH = 50;
const MIN_SHOT_DURATION_TO_SAVE_IN_HISTORY = 14000; // msec

const EMPTY_SNAPSHOT:ShotSnapshot = {
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
};

function isDatapointOutOfOrder(
  previousShotDatapoint: ShotSnapshot,
  newShotDatapoint: ShotSnapshot,
  tolerance = OUT_OF_ORDER_BUFFER_TOLERANCE,
) {
  return newShotDatapoint.timeInShot > previousShotDatapoint.timeInShot - tolerance
  && newShotDatapoint.timeInShot < previousShotDatapoint.timeInShot;
}

export function isShotLongEnoughToBeStored(shot: Shot): boolean {
  const lastDatapoint = shot.datapoints[shot.datapoints.length - 1];
  return lastDatapoint && lastDatapoint.timeInShot > MIN_SHOT_DURATION_TO_SAVE_IN_HISTORY;
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
        latestShotDatapoint: EMPTY_SNAPSHOT,
        shotRunning: false,
        currentShot: { time: Date.now(), datapoints: [] },
        shotHistory: [],

        startNewShot: () => set(() => ({ currentShot: { time: Date.now(), datapoints: [] } })),

        addShotToHistory: (shot:Shot) => set((state) => {
          if (!isShotLongEnoughToBeStored(shot)) {
            return {};
          }
          if (state.shotHistory.find((s) => s.time === shot.time)) {
            return {};
          }

          const updatedShotHistory = [...state.shotHistory, shot];
          while (updatedShotHistory.length > MAX_SHOT_HISTORY_LENGTH) {
            updatedShotHistory.shift(); // This will remove the first element from the array
          }
          return { shotHistory: updatedShotHistory };
        }),

        removeShotFromHistory: (shot: Shot) => set((state) => {
          const shotIndex = state.shotHistory.indexOf(shot);
          if (shotIndex !== -1) {
            const updatedShotHistory = [...state.shotHistory];
            updatedShotHistory.splice(shotIndex, 1);
            return { shotHistory: updatedShotHistory };
          }
          return {};
        }),

        addShotDatapoint: (shotDatapoint: ShotSnapshot) => {
          // This is here to protect the store from datapoints that are potentially delivered out of order
          // It was observed that some WS messages are delivered out of order
          if (isDatapointOutOfOrder(get().latestShotDatapoint, shotDatapoint)) {
            return;
          }

          get().setShotRunning(true);

          if (isNewShotStarted(get().currentShot, shotDatapoint)) {
            get().addShotToHistory(get().currentShot);
            get().startNewShot();
          }
          const { currentShot } = get();
          const shot = { ...(currentShot) };
          shot.datapoints.push(shotDatapoint);
          set(() => ({ currentShot: shot, latestShotDatapoint: shotDatapoint }));
        },

        setShotRunning: (value: boolean) => {
          const previousValue = get().shotRunning;

          if (previousValue === value) return;
          set(() => ({ shotRunning: value }));

          // Shot just stopped. Add to history
          if (previousValue && !value) {
            get().addShotToHistory(get().currentShot);
          }
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
