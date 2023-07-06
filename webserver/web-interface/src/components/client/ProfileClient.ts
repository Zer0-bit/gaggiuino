import axios from 'axios';
import { Profile, ProfileSummary } from '../../models/profile';

export async function getActiveProfile(): Promise<Profile> {
  return axios.get('/api/profiles/active-profile')
    .then(({ data }) => data);
}

export async function getAvailableProfiles(): Promise<Array<ProfileSummary>> {
  return axios.get('/api/profile-summaries')
    .then(({ data }) => data);
}

export async function selectActiveProfile(id: number): Promise<void> {
  return axios.put('/api/profiles/active-profile/id', { id });
}

export async function persistActiveProfileChanges(): Promise<void> {
  return axios.put('/api/profiles/active-profile/persist');
}

export async function updateActiveProfile(profile: Profile): Promise<void> {
  return axios.put('/api/profiles/active-profile', profile);
}
