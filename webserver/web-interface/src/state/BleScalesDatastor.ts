import { create } from 'zustand';
import { devtools } from 'zustand/middleware';
import { BleScales } from '../models/models';
import { getConnectedBleScales } from '../components/client/BleScalesClient';

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

// Fetching settings the first time if they aren't already loaded
const { bleScales, updateBleScales } = useBleScalesStore.getState();
if (bleScales.name.length === 0) {
  getConnectedBleScales().then(updateBleScales);
}

export default useBleScalesStore;
