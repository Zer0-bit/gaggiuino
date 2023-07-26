import axios, { AxiosRequestConfig } from 'axios';
import { Profile, ProfileSummary } from '../../models/profile';

const config:AxiosRequestConfig = {
  timeout: 4000,
};

export async function getActiveProfile(): Promise<Profile> {
  return axios.get('/api/profiles/active-profile', config)
    .then(({ data }) => data);
}

export async function getAvailableProfiles(): Promise<Array<ProfileSummary>> {
  return axios.get('/api/profile-summaries', config)
    .then(({ data }) => data);
}

export async function selectActiveProfile(id: number): Promise<void> {
  return axios.put('/api/profiles/active-profile/id', { id }, config);
}

export async function persistActiveProfileChanges(): Promise<void> {
  return axios.put('/api/profiles/active-profile/persist', config);
}

export async function updateActiveProfile(profile: Profile): Promise<void> {
  return axios.put('/api/profiles/active-profile', profile, config);
}

export async function getProfileById(id: number): Promise<Profile> {
  return axios.get(`/api/profiles/${id}`, config).then(({ data }) => data);
}

export async function createProfile(profile: Profile): Promise<Profile> {
  return axios.post('/api/profiles', profile, config)
    .then(({ data }) => data);
}

export async function deleteProfileById(id: number): Promise<void> {
  return axios.delete(`/api/profiles/${id}`, config);
}

export async function updateProfile(profile: Profile): Promise<void> {
  if (!profile.id) {
    throw Error('Attempting to update a profile without id. Create a profile first.');
  }
  return axios.put(`/api/profiles/${profile.id}`, profile, config);
}
