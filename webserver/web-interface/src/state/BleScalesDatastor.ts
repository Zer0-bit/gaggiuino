import { create } from 'zustand';
import { devtools } from 'zustand/middleware';
import { BleScales } from '../models/models';

interface BleScalesStore {
  bleScales: BleScales,
  updateBleScales: (newScales: BleScales) => void,
}

const useBleScalesStore = create<BleScalesStore>()(
  devtools(
    (set) => ({
      bleScales: {
        name: '',
        address: '',
      },
      updateBleScales: (newScales: BleScales) => set(() => ({ bleScales: newScales })),
    }),
  ),
);

export default useBleScalesStore;
