import axios from 'axios';
import { GaggiaSettings } from '../../models/models';

export async function getSettings(): Promise<GaggiaSettings> {
  return axios.get('/api/settings')
    .then(({ data }) => data);
}

export async function updateSettings(settings: GaggiaSettings): Promise<GaggiaSettings> {
  return axios.put('/api/settings', settings)
    .then(({ data }) => data);
}

export async function persistSettings(): Promise<void> {
  return axios.put('/api/settings/persist');
}
