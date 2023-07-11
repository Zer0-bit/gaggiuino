import { create } from 'zustand';
import { devtools } from 'zustand/middleware';
import { GaggiaSettings } from '../models/models';
import {
  getSettings as apiGetSettings,
  updateSettings as apiUpdateSettings,
  persistSettings as apiPersistSettings,
} from '../components/client/SettingsClient';

interface SettingsStore {
  loading: boolean;
  settings?: GaggiaSettings | undefined,
  updateLocalSettings: (settings: GaggiaSettings) => void;
  updateSettingsAndSync: (settings: GaggiaSettings) => Promise<void>;
  fetchSettings: () => Promise<void>;
  persistSettings: () => Promise<void>;
}

const useSettingsStore = create<SettingsStore>()(
  devtools((set, get) => ({
    loading: false,
    settings: undefined,
    updateLocalSettings: (settings: GaggiaSettings) => set({ settings }),
    updateSettingsAndSync: async (settings: GaggiaSettings) => {
      set({ settings });
      await apiUpdateSettings(settings);
    },
    fetchSettings: async () => {
      if (get().loading) return;
      set({ loading: true });
      set({ settings: await apiGetSettings(), loading: false });
    },
    persistSettings: async () => {
      await apiPersistSettings();
    },
  })),
);

// Fetching settings the first time if they aren't already loaded
const { loading, settings, fetchSettings } = useSettingsStore.getState();
if (!settings && !loading) {
  fetchSettings();
}

export default useSettingsStore;
