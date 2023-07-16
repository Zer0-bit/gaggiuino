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

export async function getProfileById(id: number): Promise<Profile> {
  return axios.get(`/api/profiles/${id}`).then(({ data }) => data);
}

export async function createProfile(profile: Profile): Promise<Profile> {
  return axios.post('/api/profiles', profile)
    .then(({ data }) => data);
}

export async function deleteProfileById(id: number): Promise<void> {
  return axios.delete(`/api/profiles/${id}`);
}

export async function updateProfile(profile: Profile): Promise<void> {
  if (!profile.id) {
    throw Error('Attempting to update a profile without id. Create a profile first.');
  }
  return axios.put(`/api/profiles/${profile.id}`, profile);
}
