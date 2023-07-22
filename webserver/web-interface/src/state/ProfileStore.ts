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
  availableProfiles: ProfileSummary[],
  updateLocalActiveProfile: (profile: Profile) => void,
  updateActiveProfileAndSync: (profile: Profile) => Promise<void>,
  fetchActiveProfile: () => Promise<void>,
  fetchAvailableProfiles: () => Promise<void>,
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
      updateLocalActiveProfile: (profile: Profile) => {
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
      updateActiveProfileAndSync: async (profile: Profile) => {
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

// Fetching activeProfile the first time if they aren't already loaded
const { activeProfileLoading, activeProfile, fetchActiveProfile } = useProfileStore.getState();
if (!activeProfile && !activeProfileLoading) {
  fetchActiveProfile();
}

// Fetching availableProfiles the first time if they aren't already loaded
const { availableProfilesLoading, availableProfiles, fetchAvailableProfiles } = useProfileStore.getState();
if (availableProfiles.length === 0 && !availableProfilesLoading) {
  fetchAvailableProfiles();
}

export default useProfileStore;
