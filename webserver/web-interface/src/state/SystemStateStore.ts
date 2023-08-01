import { create } from 'zustand';
import { devtools } from 'zustand/middleware';
import { OperationMode, SystemState } from '../models/models';

interface SystemStateStore {
  systemState: SystemState,
  updateLocalSystemState: (newState: SystemState) => void,
  updateLocalOperationMode: (newOpMode: OperationMode) => void,
  updateTarePending: (newState: boolean) => void,
}

const useSystemStateStore = create<SystemStateStore>()(
  devtools(
    (set) => ({
      systemState: {
        startupInitFinished: false,
        operationMode: OperationMode.BREW_AUTO,
        tofReady: false,
        isSteamForgottenON: false,
        scalesPresent: false,
        timeAlive: 0,
        descaleProgress: 0,
        tarePending: false,
      },
      updateLocalSystemState: (newState: SystemState) => set(() => ({ systemState: newState })),
      updateLocalOperationMode: (newOpMode: OperationMode) => set((state) => ({
        systemState: { ...state.systemState, operationMode: newOpMode },
      })),
      updateTarePending: (newState: boolean) => set((state) => ({
        systemState: { ...state.systemState, tarePending: newState },
      })),
    }),
  ),
);

export default useSystemStateStore;
