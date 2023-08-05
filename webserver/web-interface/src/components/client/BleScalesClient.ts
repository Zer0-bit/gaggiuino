import axios, { AxiosRequestConfig } from 'axios';
import { BleScales } from '../../models/models';

const config:AxiosRequestConfig = {
  timeout: 4000,
};

export async function getConnectedBleScales(): Promise<BleScales> {
  return axios.get('/api/bt-scales/connected', config)
    .then(({ data }) => data);
}

export async function getAvailableBleScales(): Promise<BleScales[]> {
  return axios.get('/api/bt-scales/available', config)
    .then(({ data }) => data);
}
