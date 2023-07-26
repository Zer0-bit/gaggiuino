import axios, { AxiosRequestConfig } from 'axios';
import { GaggiaSettings } from '../../models/models';

const config:AxiosRequestConfig = {
  timeout: 4000,
};

export async function getSettings(): Promise<GaggiaSettings> {
  return axios.get('/api/settings', config)
    .then(({ data }) => data);
}

export async function updateSettings(settings: GaggiaSettings): Promise<GaggiaSettings> {
  return axios.put('/api/settings', settings, config)
    .then(({ data }) => data);
}

export async function persistSettings(): Promise<void> {
  return axios.put('/api/settings/persist', config);
}
