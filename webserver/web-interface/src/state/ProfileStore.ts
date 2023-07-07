import { create } from 'zustand';
import { devtools } from 'zustand/middleware';
import { Profile, ProfileSummary } from '../models/profile';
import {
  getActiveProfile, getAvailableProfiles, persistActiveProfileChanges, selectActiveProfile, updateActiveProfile,
} from '../components/client/ProfileClient';

interface ProfileStore {
  activeProfileLoading: boolean,
  availableProfilesLoading: boolean,
  activeProfile: Profile | null,
  availableProfiles: Array<ProfileSummary>,
  setLocalActiveProfile: (profile: Profile) => void,
  fetchActiveProfile: () => Promise<void>,
  fetchAvailableProfiles: () => Promise<void>,
  updateActiveProfile: (profile: Profile) => Promise<void>,
  persistActiveProfile: () => Promise<void>,
  selectNewProfile: (id: number) => Promise<void>,
}

const useProfileStore = create<ProfileStore>()(
  devtools(
    (set) => ({
      activeProfileLoading: false,
      availableProfilesLoading: false,
      activeProfile: null,
      availableProfiles: [],
      setLocalActiveProfile: (profile: Profile) => {
        set({ activeProfile: profile });
      },
      fetchActiveProfile: async () => {
        set({ activeProfileLoading: true });
        set({ activeProfile: await getActiveProfile(), activeProfileLoading: false });
      },
      fetchAvailableProfiles: async () => {
        set({ availableProfilesLoading: true });
        set({ availableProfiles: await getAvailableProfiles(), availableProfilesLoading: false });
      },
      updateActiveProfile: async (profile: Profile) => {
        await updateActiveProfile(profile);
        set({ activeProfile: profile });
      },
      persistActiveProfile: async () => { persistActiveProfileChanges(); },
      selectNewProfile: async (id: number) => {
        set({ activeProfileLoading: true });
        await selectActiveProfile(id);
        set({ activeProfile: await getActiveProfile(), activeProfileLoading: false });
      },
    }),
  ),
);

export default useProfileStore;
