import { create } from 'zustand';
import { devtools } from 'zustand/middleware';
import { DescalingProgress, DescalingState } from '../models/models';

interface DescalingProgressStore {
  descalingProgress: DescalingProgress,
  updateLocalDescalingProgress: (newState: DescalingProgress) => void,
}

const useDescalingProgressStore = create<DescalingProgressStore>()(
  devtools(
    (set) => ({
      descalingProgress: {
        state: DescalingState.IDLE,
        progress: 0,
        time: 0,
      },
      updateLocalDescalingProgress: (descalingProgress: DescalingProgress) => set(() => ({ descalingProgress })),
    }),
  ),
);

export default useDescalingProgressStore;
